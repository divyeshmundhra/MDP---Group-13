import argparse
import time
import numpy as np
#import pyautogui ##testing for not responding stream window
from pathlib import Path

import os
import cv2
import torch
import torch.backends.cudnn as cudnn

from collections import Counter
from models.experimental import attempt_load
from utils.datasets import LoadStreams, LoadImages, LoadImagesZMQ
from utils.general import check_img_size, non_max_suppression, apply_classifier, scale_coords, xyxy2xywh, strip_optimizer, set_logging, increment_path
from utils.plots import plot_one_box
from utils.torch_utils import select_device, load_classifier, time_synchronized

import zmq
import json

context = zmq.Context()
tx = context.socket(zmq.PUSH)
tx.connect("tcp://192.168.13.1:3001")


def detect(save_img=True):
    source, weights, view_img, save_txt, imgsz = opt.source, opt.weights, opt.view_img, opt.save_txt, opt.img_size
    image_id = {'0':'10', '1':'6', '2':'7', '3':'8', '4':'9', '5':'5', '6':'2', '7':'4', '8':'3', '9':'1', '10':'11', '11':'12', '12':'13', '13':'14', '14':'15'}
    #image_id maps the Roboflow ids to the actual ids
    robo_id_list = list(map(str, range(1, 16)))  #used with Roboflow ids, not the actual ids
    
    bimodal_img_loc_dict = {str(i) : [] for i in range(1, 16)}      #create dict for storing locations for each id
    

    webcam = source.isnumeric() or source.endswith('.txt') or source.lower().startswith(
        ('rtsp://', 'rtmp://', 'http://'))

    # Directories
    save_dir = Path(increment_path(Path(opt.project) / opt.name, exist_ok=opt.exist_ok))  # increment run
    save_dir.mkdir(parents=True, exist_ok=True)  # make dir

    # Initialize
    set_logging()
    device = select_device(opt.device)
    half = device.type != 'cpu'  # half precision only supported on CUDA

    # Load model
    model = attempt_load(weights, map_location=device)  # load FP32 model
    imgsz = check_img_size(imgsz, s=model.stride.max())  # check img_size
    if half:
        model.half()  # to FP16

    # Second-stage classifier
    classify = False
    if classify:
        modelc = load_classifier(name='resnet101', n=2)  # initialize
        modelc.load_state_dict(torch.load('weights/resnet101.pt', map_location=device)['model']).to(device).eval()

    # Set Dataloader
    vid_path, vid_writer = None, None
    if webcam:
        view_img = True
        cudnn.benchmark = True  # set True to speed up constant image size inference
        dataset = LoadStreams(source, img_size=imgsz)
    elif source.startswith("tcp://"):
        view_img = True
        dataset = LoadImagesZMQ(source, img_size=imgsz)
    else:
        save_img = True
        dataset = LoadImages(source, img_size=imgsz)

    # Get names and colors
    names = model.module.names if hasattr(model, 'module') else model.names
    colors = [[np.random.randint(0, 255) for _ in range(3)] for _ in names]

    # Run inference
    t0 = time.time()
    img = torch.zeros((1, 3, imgsz, imgsz), device=device)  # init img
    _ = model(img.half() if half else img) if device.type != 'cpu' else None  # run once
    
    num = 0
    loc_list = []
    
    for robot_info_str, img, im0s, vid_cap in dataset:
        if source.startswith("tcp://"):
            robot_info = json.loads(robot_info_str)
        else:
            robot_info = {"x":2, "y": 2, "orientation": "NORTH"}    #dummy 
        
        img = torch.from_numpy(img).to(device)
        img = img.half() if half else img.float()  # uint8 to fp16/32
        img /= 255.0  # 0 - 255 to 0.0 - 1.0
        if img.ndimension() == 3:
            img = img.unsqueeze(0)

        # Inference
        t1 = time_synchronized()
        pred = model(img, augment=opt.augment)[0]

        # Apply NMS
        pred = non_max_suppression(pred, opt.conf_thres, opt.iou_thres, classes=opt.classes, agnostic=opt.agnostic_nms)
        t2 = time_synchronized()

        # Apply Classifier
        if classify:
            pred = apply_classifier(pred, modelc, img, im0s)
            
        # finish if detected all images
        #if num == 5:                          
        #            print("finished all detections")
        #            return
        # Process detections
        
        for i, det in enumerate(pred):  # detections per image
            if webcam:  # batch_size >= 1
                s, im0 = '%g: ' % i, im0s[i].copy()
            else:
                s, im0 = '', im0s

            #save_path = str(save_dir)
            save_path = os.path.join(os.getcwd(),'runs\\exp\\')
            txt_path = str(save_dir) + ('_%g' % dataset.frame if dataset.mode == 'video' else '')


            s += '%gx%g ' % img.shape[2:]  # print string
            gn = torch.tensor(im0.shape)[[1, 0, 1, 0]]  # normalization gain whwh
            if len(det):
                # Rescale boxes from img_size to im0 size
                det[:, :4] = scale_coords(img.shape[2:], det[:, :4], im0.shape).round()
                    
                # Print results
                for c in det[:, -1].unique():
                    n = (det[:, -1] == c).sum()  # detections per class
                    s += '%g %s, ' % (n, names[int(c)])  # add to string

                # Write results
                for *xyxy, conf, cls in reversed(det):
                    
                    distance = 2
                
                    if save_txt:  # Write to file
                        xywh = (xyxy2xywh(torch.tensor(xyxy).view(1, 4)) / gn).view(-1).tolist()  # normalized xywh
                        line = (cls, *xywh, conf) if opt.save_conf else (cls, *xywh)  # label format
                        
                        bbox_label = ((('%g ' * len(line)).rstrip() % line).split()) # 0:ID, 1:x, 2:y, 3:w, 4:h
                        img_id, img_x, img_y, img_w, img_h = bbox_label[0], float(bbox_label[1]), float(bbox_label[2]), float(bbox_label[3]), float(bbox_label[4])
                        bimodial_img_loc_list = bimodal_img_loc_dict[image_id[img_id]]
                        
                        with open(txt_path + '.txt', 'a') as f:
                            f.write('id:%s | x:%s | y:%s | w:%s | h:%s | locx:%d | locy:%d'%(image_id[img_id], img_x, img_y, img_w, img_h, robot_info['x'], robot_info['y'])+'\n')

                    if save_img or view_img:  # Add bbox to image
                    
                        if img_w < 0.12 and img_h < 0.21:       #finetune estimation!!
                            distance = 3
                        
                        # partitioning image 
                        
                        if(img_x<=0.4):
                            print("%d, %d : image %s ahead by %d distance"%(robot_info['x'], robot_info['y'], image_id[img_id], distance))
                            loc = image_ahead(robot_info, distance)
                        
                        elif(0.4<img_x<0.6):
                            print("%d, %d : image %s beside by %d distance"%(robot_info['x'], robot_info['y'], image_id[img_id], distance))
                            loc = image_beside(robot_info, distance)
                            
                        else:
                            print("%d, %d : image %s behind by %d distance"%(robot_info['x'], robot_info['y'], image_id[img_id], distance))
                            loc = image_behind(robot_info, distance)
                       
                        
                        label = '%s %f %.3f %.3f' % (image_id[img_id], conf, img_x, img_y)
                        plot_one_box(xyxy, im0, label=label, color=colors[int(cls)], line_thickness=3)
                        
                        
                        
                        if(len(bimodial_img_loc_list) == 0):            #take only 1 photo per id    
                            cv2.imwrite(save_path+str(num)+".jpg", im0)
                            num+=1
                                
                        bimodial_img_loc_list.append((loc['x'], loc['y']))  
                        #print(bimodial_img_loc_list)
                        bimodial_tuple = tuple(bimodial_img_loc_list)   #convert list to tuple, Counter needs hashable
                        bimod_x, bimod_y = Counter(bimodial_tuple).most_common()[0][0][0], Counter(bimodial_tuple).most_common()[0][0][1]   #find most occuring (x, y)
                        print(bimod_x, bimod_y)
                        
                        
                            #send image location
                        data = {'id':image_id[img_id], 'x':bimod_x, 'y':bimod_y}
                        send_time = str(time.time())
                        #if(len(bimodial_img_loc_list)%2!=0):
                        tx.send_json({"type": "detection", "data": data, "id": send_time})
                            
                        with open(txt_path + '.txt', 'a') as f:
                            f.write('%s locx:%d locy:%d'%(send_time, bimod_x, bimod_y) +'\n\n')
                        
            

            # Print time (inference + NMS)
            #print('%sNothing detected (%.3fs)' % (s, t2 - t1))

            # Stream results
            if view_img:
                cv2.imshow(str('image'), im0)
                if cv2.waitKey(1) == ord('q'):  # q to quit
                    return
                    #raise StopIteration
            

            # Save results (image with detections)
            #if save_img:
            #    if dataset.mode == 'images':
            #        cv2.imwrite(save_path+".jpg", im0)
            #    else:
            #        if vid_path != save_path:  # new video
            #            vid_path = save_path
            #            if isinstance(vid_writer, cv2.VideoWriter):
            #                vid_writer.release()  # release previous video writer

            #           fourcc = 'mp4v'  # output video codec
            #            fps = vid_cap.get(cv2.CAP_PROP_FPS)
            #            w = int(vid_cap.get(cv2.CAP_PROP_FRAME_WIDTH))
            #            h = int(vid_cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
            #            vid_writer = cv2.VideoWriter(save_path, cv2.VideoWriter_fourcc(*fourcc), fps, (w, h))
            #        vid_writer.write(im0)

    #if save_txt or save_img:
    #    s = f"\n{len(list(save_dir.glob('*.txt')))} labels saved to save_dir" if save_txt else ''
    #    print(f"Results saved to {save_dir}{s}")

    #print('Done. (%.3fs)' % (time.time() - t0))

#function to update location (based on distance?no)
def image_beside(loc, distance):
    
    if loc['orientation'] == 'NORTH':
        loc['x'] += distance
        return loc
    if loc['orientation'] == 'SOUTH':
        loc['x'] -= distance
        return loc
    if loc['orientation'] == 'WEST':
        loc['y'] += distance
        return loc
    if loc['orientation'] == 'EAST':
        loc['y'] -= distance
        return loc
        
def image_ahead(loc, distance):
    if loc['orientation'] == 'NORTH':
        loc['x'] += distance
        loc['y'] += 1
        return loc
    if loc['orientation'] == 'SOUTH':
        loc['x'] -= distance
        loc['y'] -= 1
        return loc
    if loc['orientation'] == 'WEST':
        loc['y'] += distance
        loc['x'] -= 1
        return loc
    if loc['orientation'] == 'EAST':
        loc['y'] -= distance
        loc['x'] += 1
        return loc
    
def image_behind(loc, distance):
    if loc['orientation'] == 'NORTH':
        loc['x'] += distance
        loc['y'] -= 1
        return loc
    if loc['orientation'] == 'SOUTH':
        loc['x'] -= distance
        loc['y'] += 1
        return loc
    if loc['orientation'] == 'WEST':
        loc['y'] += distance
        loc['x'] += 1
        return loc
    if loc['orientation'] == 'EAST':
        loc['y'] -= distance
        loc['x'] -= 1
        return loc

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--weights', nargs='+', type=str, default='yolov5s.pt', help='model.pt path(s)')
    parser.add_argument('--source', type=str, default='data/images', help='source')  # file/folder, 0 for webcam
    parser.add_argument('--img-size', type=int, default=640, help='inference size (pixels)')
    parser.add_argument('--conf-thres', type=float, default=0.25, help='object confidence threshold')
    parser.add_argument('--iou-thres', type=float, default=0.45, help='IOU threshold for NMS')
    parser.add_argument('--device', default='', help='cuda device, i.e. 0 or 0,1,2,3 or cpu')
    parser.add_argument('--view-img', action='store_true', help='display results')
    parser.add_argument('--save-txt', action='store_true', help='save results to *.txt')
    parser.add_argument('--save-conf', action='store_true', help='save confidences in --save-txt labels')
    parser.add_argument('--classes', nargs='+', type=int, help='filter by class: --class 0, or --class 0 2 3')
    parser.add_argument('--agnostic-nms', action='store_true', help='class-agnostic NMS')
    parser.add_argument('--augment', action='store_true', help='augmented inference')
    parser.add_argument('--update', action='store_true', help='update all models')
    parser.add_argument('--project', default='runs', help='save results to project/name')
    parser.add_argument('--name', default='exp', help='save results to project/name')
    parser.add_argument('--exist-ok', action='store_true', help='existing project/name ok, do not increment')
    opt = parser.parse_args()
    print(opt)
    
    with torch.no_grad():
        count = 0
        im_path = os.path.join(os.getcwd(),'runs\exp')
        if opt.update:  # update all models (to fix SourceChangeWarning)
            for opt.weights in ['yolov5s.pt', 'yolov5m.pt', 'yolov5l.pt', 'yolov5x.pt']:
                detect()
                strip_optimizer(opt.weights)
        else:
            try:
                detect()
            except KeyboardInterrupt as e:
            # Display taken pictures
                im = cv2.imread(os.path.join(im_path, '0.jpg'))
                imstack = cv2.resize(im, (320, 416))    #initialise photo stack
                for x in os.listdir(im_path):
                    if count == 0:
                        count += 1
                        continue
                    im = cv2.imread(os.path.join(im_path+"\\"+x))
                    im = cv2.resize(im, (320, 416))
                    imstack = np.hstack((imstack, im))
                cv2.imshow('images', imstack)
                cv2.waitKey(0)
#deal with not responding video leading to not showing the images upon finish