#!/usr/bin/python
import numpy as np
import cv2
import argparse
import dewarp
import feature_matching
import optimal_seamline
import blending
import cropping
import os

import sys

# --------------------------------
dir_path = os.path.dirname(os.path.realpath(__file__))
cwd = os.getcwd()
# --------------------------------


def Hcalc(cap, xmap, ymap, W, H, W_remap, offsetYL, offsetYR, templ_shape, maxL, maxR):
    """Calculate and return homography for stitching process."""
    Mlist = []
    frame_count = cap.get(cv2.CAP_PROP_FRAME_COUNT)

    for frame_no in np.arange(0, frame_count, int(frame_count / 10)):
        cap.set(cv2.CAP_PROP_POS_FRAMES, frame_no)
        ret, frame = cap.read()
        if ret:
            # defish / unwarp
            cam1 = cv2.remap(frame[:, :H], xmap, ymap, cv2.INTER_LINEAR)
            cam2 = cv2.remap(frame[:, H:], xmap, ymap, cv2.INTER_LINEAR)
            cam1_gray = cv2.cvtColor(cam1, cv2.COLOR_BGR2GRAY)
            cam2_gray = cv2.cvtColor(cam2, cv2.COLOR_BGR2GRAY)

            # shift the remapped images along x-axis
            shifted_cams = np.zeros((H * 2, W, 3), np.uint8)
            shifted_cams[H:, (W - W_remap) / 2:(W + W_remap) / 2] = cam2
            shifted_cams[:H, :W_remap / 2] = cam1[:, W_remap / 2:]
            shifted_cams[:H, W - W_remap / 2:] = cam1[:, :W_remap / 2]

            # find matches and extract pairs of correspondent matching points
            matchesL = feature_matching.getMatches_goodtemplmatch(
                cam1_gray[offsetYL:H - offsetYL, W / 2:],
                cam2_gray[offsetYL:H - offsetYL, :W_remap - W / 2],
                templ_shape, maxL)
            matchesR = feature_matching.getMatches_goodtemplmatch(
                cam2_gray[offsetYR:H - offsetYR, W / 2:],
                cam1_gray[offsetYR:H - offsetYR, :W_remap - W / 2],
                templ_shape, maxR)
            matchesR = matchesR[:, -1::-1]

            matchesL = matchesL + ((W - W_remap) / 2, offsetYL)
            matchesR = matchesR + ((W - W_remap) / 2 + W / 2, offsetYR)
            zipped_matches = zip(matchesL, matchesR)
            matches = np.int32([e for i in zipped_matches for e in i])
            pts1 = matches[:, 0]
            pts2 = matches[:, 1]

            # find homography from pairs of correspondent matchings
            M, status = cv2.findHomography(pts2, pts1, cv2.RANSAC, 4.0)
            Mlist.append(M)
    M = np.average(np.array(Mlist), axis=0)
    # print M
    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
    return M


def main(input, output, input_width, number_of_frames, no_stdout):

    W = input_width
    H = input_width / 2
    W_remap = input_width/2 + 100

    # --------------------------------
    # output video resolution
    # W = 2560
    # H = W/2
    # --------------------------------
    # field of view, width of de-warped image
    FOV = 194.0
    W_remap = W/2 + 100
    # --------------------------------
    # params for template matching
    templ_shape = (60, 16)
    offsetYL = 160
    offsetYR = 160
    maxL = 80
    maxR = 80
    # --------------------------------
    # params for optimal seamline and multi-band blending
    W_lbl = 120
    blend_level = 3

    cap = cv2.VideoCapture(input)

    # define the codec and create VideoWriter object
    out = None

    if output != "": 
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')    
        out = cv2.VideoWriter(output, fourcc, 30.0, (W, H))

    # obtain xmap and ymap
    xmap, ymap = dewarp.buildmap(Ws=W_remap, Hs=H, Wd=H, Hd=H, fov=FOV)

    # calculate homography
    M = Hcalc(cap, xmap, ymap, W, H, W_remap, offsetYL, offsetYR, templ_shape, maxL, maxR)

    # calculate vertical boundary of warped image, for later cropping
    top, bottom = cropping.verticalBoundary(M, W_remap, W, H)

    # estimate empty (invalid) area of warped2
    EAof2 = np.zeros((H, W, 3), np.uint8)
    EAof2[:, (W - W_remap) / 2 + 1:(W + W_remap) / 2 - 1] = 255
    EAof2 = cv2.warpPerspective(EAof2, M, (W, H))

    # process the first frame
    ret, frame = cap.read()
    if ret:
        # de-warp
        cam1 = cv2.remap(frame[:, :H], xmap, ymap, cv2.INTER_LINEAR)
        cam2 = cv2.remap(frame[:, H:], xmap, ymap, cv2.INTER_LINEAR)

        # shift the remapped images along x-axis
        shifted_cams = np.zeros((H * 2, W, 3), np.uint8)
        shifted_cams[H:, (W - W_remap) / 2:(W + W_remap) / 2] = cam2
        shifted_cams[:H, :W_remap / 2] = cam1[:, W_remap / 2:]
        shifted_cams[:H, W - W_remap / 2:] = cam1[:, :W_remap / 2]

        # warp cam2 using homography M
        warped2 = cv2.warpPerspective(shifted_cams[H:], M, (W, H))
        warped1 = shifted_cams[:H]

        # crop to get a largest rectangle, and resize to maintain resolution
        warped1 = cv2.resize(warped1[top:bottom], (W, H))
        warped2 = cv2.resize(warped2[top:bottom], (W, H))

        # image labeling (find minimum error boundary cut)
        mask, minloc_old = optimal_seamline.imgLabeling(
            warped1[:, W_remap / 2 - W_lbl:W_remap / 2],
            warped2[:, W_remap / 2 - W_lbl:W_remap / 2],
            warped1[:, W - W_remap / 2:W - W_remap / 2 + W_lbl],
            warped2[:, W - W_remap / 2:W - W_remap / 2 + W_lbl],
            (W, H), W_remap / 2 - W_lbl, W - W_remap / 2)

        labeled = warped1 * mask + warped2 * (1 - mask)

        # fill empty area of warped1 and warped2, to avoid darkening
        warped1[:, W_remap / 2:W - W_remap /
                2] = warped2[:, W_remap / 2:W - W_remap / 2]
        warped2[EAof2 == 0] = warped1[EAof2 == 0]

        # multi band blending
        blended = blending.multi_band_blending(
            warped1, warped2, mask, blend_level)

        # cv2.imshow('p', blended.astype(np.uint8))
        # cv2.waitKey(0)

        # write results from phases
        if out:
            out.write(blended.astype(np.uint8))

        if no_stdout == False:
            img_yuv = cv2.cvtColor(blended.astype(np.uint8), cv2.COLOR_BGR2YUV)
            y, u, v = cv2.split(img_yuv)

            sys.stdout.write(y.astype(np.uint8))
            sys.stdout.write(u.astype(np.uint8))
            sys.stdout.write(v.astype(np.uint8))

    count = 0

    # process each frame
    while(cap.isOpened()):
        ret, frame = cap.read()
        if ret:
            # de-warp
            cam1 = cv2.remap(frame[:, :H], xmap, ymap, cv2.INTER_LINEAR)
            cam2 = cv2.remap(frame[:, H:], xmap, ymap, cv2.INTER_LINEAR)

            # shift the remapped images along x-axis
            shifted_cams = np.zeros((H * 2, W, 3), np.uint8)
            shifted_cams[H:, (W - W_remap) / 2:(W + W_remap) / 2] = cam2
            shifted_cams[:H, :W_remap / 2] = cam1[:, W_remap / 2:]
            shifted_cams[:H, W - W_remap / 2:] = cam1[:, :W_remap / 2]

            # warp cam2 using homography M
            warped2 = cv2.warpPerspective(shifted_cams[H:], M, (W, H))
            warped1 = shifted_cams[:H]

            # crop to get a largest rectangle
            # and resize to maintain resolution
            warped1 = cv2.resize(warped1[top:bottom], (W, H))
            warped2 = cv2.resize(warped2[top:bottom], (W, H))

            # image labeling (find minimum error boundary cut)
            mask, minloc_old = optimal_seamline.imgLabeling(
                warped1[:, W_remap / 2 - W_lbl:W_remap / 2],
                warped2[:, W_remap / 2 - W_lbl:W_remap / 2],
                warped1[:, W - W_remap / 2:W - W_remap / 2 + W_lbl],
                warped2[:, W - W_remap / 2:W - W_remap / 2 + W_lbl],
                (W, H), W_remap / 2 - W_lbl, W - W_remap / 2, minloc_old)

            labeled = warped1 * mask + warped2 * (1 - mask)

            # fill empty area of warped1 and warped2, to avoid darkening
            warped1[:, W_remap / 2:W - W_remap /
                    2] = warped2[:, W_remap / 2:W - W_remap / 2]
            warped2[EAof2 == 0] = warped1[EAof2 == 0]

            # multi band blending
            blended = blending.multi_band_blending(
                warped1, warped2, mask, blend_level)

            sys.stderr.write("Processing frame " + str(count) + "\n")

            count = count + 1

            if count == number_of_frames and number_of_frames != -1:
                break;

            # write the remapped frame
            if out:
                out.write(blended.astype(np.uint8))
            
            if no_stdout == False:
                img_yuv = cv2.cvtColor(blended.astype(np.uint8), cv2.COLOR_BGR2YUV)
                y, u, v = cv2.split(img_yuv)

                sys.stdout.write(y.astype(np.uint8))
                sys.stdout.write(u.astype(np.uint8))
                sys.stdout.write(v.astype(np.uint8))

        else:
            break

    # release everything if job is finished
    cap.release()
    
    if out:
        out.release()

    cv2.destroyAllWindows()


if __name__ == '__main__':
    # construct the argument parse and parse the arguments
    ap = argparse.ArgumentParser(
        description="A summer research project to seamlessly stitch \
                     dual-fisheye video into 360-degree videos")
    ap.add_argument('-n', '--number_of_frames', type=int, default=-1,
                    help="total number of frames to process")
    ap.add_argument('input_width', type=int,
                    help="width of input dual fisheye video")
    ap.add_argument('--input', default="/dev/stdin",
                    help="path to the input dual fisheye video")
    ap.add_argument('-o', '--output', default="",
                    help="path to the output stitched video")
    ap.add_argument('-s', '--no_stdout', action="store_true",
                    help="do not write output to standard output")

    args = vars(ap.parse_args())
    main(args['input'], args['output'], args['input_width'], args['number_of_frames'], args['no_stdout'])


