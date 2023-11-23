# 카카오 미러서버 설정(속도 빨라진다.)
    sudo vi /etc/apt/sources.list
키보드로  : (콜론)   을 입력해준다.(줄 바꿈 명령이다.)

    :
    %s/kr.archive.ubuntu.com/mirror.kakao.com/
    :
    %s/security.ubuntu.com/mirror.kakao.com/
    :wq! 
    sudo apt update



# ros melodic 설치
    sudo sh -c 'echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list'
    sudo apt install curl
    curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | sudo apt-key add -
    sudo apt update
    sudo apt install ros-melodic-desktop-full
    echo "source /opt/ros/melodic/setup.bash" >> ~/.bashrc
    source ~/.bashrc
    sudo apt install python-rosdep python-rosinstall python-rosinstall-generator python-wstool build-essential
    sudo apt install python-rosdep
    sudo rosdep init
    rosdep update


    
## catkin_ws 작업공간 설정하기
    mkdir -p ~/catkin_ws/src       
    cd ~/catkin_ws/src
    catkin_init_workspace          
    cd ~/catkin_ws/
    catkin_make         
    source devel/setup.bash    
    


# rplidar_ros 설치
    cd ~/catkin_ws/src                                               //만약 작업공간을 만들지 않았다면 "업무 #316 ros 설치법"에서 작업공간을 설정한다.
    git clone https://github.com/Slamtec/rplidar_ros.git
    cd ~/catkin_ws
    catkin_make
rplidar a2의 환경설정을 해야한다.

    sudo apt-get install expect-dev
    unbuffer udevadm monitor --environment | grep 'ID_SERIAL='
    
unbuffer 명령어를 입력하고 rplidar a2의 USB 커넥터를 PC에 연결했다 뺏다 하면 아래와 비슷한 화면이 나타난다.
![image](https://user-images.githubusercontent.com/105560901/222706668-a548e4c7-059a-4f74-afa7-710bcb6e579a.png)

ID_SERIAL= 의 뒤에 있는 내용들은 이후에 사용할 예정이므로 기억할 필요가 있다.

예시) Sliicon_Labs_CP2102_to_UART_Bridge_대충 숫자, 영어 많은 글

    cd /etc/udev/rules.d
    sudo nano 60-persistent-serial.rules
    
아래 내용을 전부 복붙한다. 단, ENV{ID_SERIAL} 의 내용은 위에서 ID_SERIAL= 뒤에 내용으로 복붙한다.
또한 SYMLINK+= 뒤에 내용은 본인의 환경에 맞춰서 다르게 입력해줄 필요가 있다.(본인의 pc에서 라이더를 연결했을때 나타나는 포트번호를 확인해야한다.)
그리고 OWNER의 내용도 본인의 사용자 이름으로 설정해야 한다.(터미널에서 나타나는 초록색 문장중에 @ 앞에 있는거, 예시 : junho@junho-TFG257GS 중에서 junho)

    ACTION!="add", GOTO="persistent_serial_end" 
    SUBSYSTEM!="tty", GOTO="persistent_serial_end" 
    KERNEL!="ttyUSB[0-9]*", GOTO="persistent_serial_end" 

    # This is old 11.10 style: IMPORT="usb_id --export %p" 
    IMPORT{builtin}="path_id" 
    ENV{ID_SERIAL}=="Silicon_Labs_CP2102_USB_to_UART_Bridge_9640aec1b4df15439b240de35d6f287e" , SYMLINK="rplidarA2"       , SYMLINK+="ttyUSB0" , OWNER="junho" 

    LABEL="persistent_serial_end" 
    
복붙했다면 [ctrl + s] , [ctrl + x] 입력한다.(저장, 나가기)


## rplidar git 내용 변경된 문제 해결법
    cd ~/catkin_ws/src/rplidar_ros
    git reset --hard 4f8ddee37de1d9b5a3a1d222607b9097c15ba444


## rplidar a2 실행

    sudo chmod 666 /dev/ttyUSB0                           //본인의 경우에 맞게 편집할것
    roscore
    roslaunch rplidar_ros view_rplidar.launch             //lidar a1과 a2의 경우임
    
### roslaunch rplidar_ros view_rplidar.launch 에러 해결법
![image](https://user-images.githubusercontent.com/105560901/222708540-a844a128-a1e4-4f74-9582-a9eb57c53d19.png)

    cd ~/catkin_ws
    catkin_make
    source devel/setup.bash
    rospack profile
    rospack find rplidar_ros                    // 이 명령어로 패키지가 찾아지면 보통은 해결된다.

### operation time out 에러 해결법
~/catkin_ws/src/rplidar_ros/launch/rplidar.launch 파일에 들어가서 serial_baudrate의 값을 115200에서 256000으로 변경해준다.
또한 라이더의 어뎁터에 물리적으로 baudrate를 결정하는 스위치가 있는데 그것도 변경해 준다.

![image](https://user-images.githubusercontent.com/105560901/222709117-a844fd21-c7c1-40a5-83fc-7a614fe95dc5.png)

Description : 우리 팀에서 사용하는 라이더는 a2이지만, 정확한 모델명은 rplidar a2m12 이다. 대부분의 a2 라이더는 115200으로 통신하지만
해당 모델의 경우(rplidar a2m12) serial_baudrate를 256000으로 사용하기 때문에 launch 파일에서 이러한 부분을 수정 할 필요가 있다.

### rplidar_sdk 설치(생략 가능)
    git clone https://github.com/Slamtec/rplidar_sdk.git    //home 디렉토리에서 설치한다.
    cd rplidar_sdk
    make                                                    // 이 명령어를 통해 output/Linux/Release 경로가 생겼을 것이다.
    cd output/Linux/Release
    ./ultra_simple       
sdk 설치에서 ultra_simple이 제대로 실행되지 않더라고 아래의 rplidar_ros 패키지를 통해 ros에서 라이더를 구동하는것에 큰 문제가 없다.

# realsense 패키지 설치
    sudo apt-get install ros-$ROS_DISTRO-realsense2-camera
    sudo apt-get install ros-$ROS_DISTRO-realsense2-description
    
## 빌드 의존성 설치
    sudo apt-get update && sudo apt-get upgrade && sudo apt-get dist-upgrade
    git clone https://github.com/IntelRealSense/librealsense.git
    cd librealsense
    sudo apt-get install git libssl-dev libusb-1.0-0-dev libudev-dev pkg-config libgtk-3-dev

우분투 버전에 따라 아래 3가지 명령어 중 선택

    // 우분투 14
    ./scripts/install_glfw3.sh              

    // 우분투 16
    sudo apt-get install libglfw3-dev

    // 우분투 18/20
    sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev at

## Intel® RealSense™ SDK 2.0 설치
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-key F6E65AC044F831AC80A06380C8B3A55A6F3EFCDE || sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-key F6E65AC044F831AC80A06380C8B3A55A6F3EFCDE
    
만약 위의 명령어로 key를 검색하지 못했다면 오른쪽의 명령어를 입력하고 다시 시도한다. export http_proxy="http://<proxy>:<port>" 

    sudo add-apt-repository "deb https://librealsense.intel.com/Debian/apt-repo $(lsb_release -cs) main" -u
    sudo apt-get install librealsense2-dkms
    sudo apt-get install librealsense2-utils
    sudo apt-get install librealsense2-dev
    sudo apt-get install librealsense2-dbg                 // SDK 설치 완료
  
## realsense camera SDK 실행
    realsense-viewer

## Intel® RealSense™ ROS 설치
    cd ~/catkin_ws/src/
    git clone https://github.com/IntelRealSense/realsense-ros.git
    cd realsense-ros/
    git checkout `git tag | sort -V | grep -P "^2.\d+\.\d+" | tail -1`
    cd ~/catkin_ws
    catkin_make clean
    catkin_make -DCATKIN_ENABLE_TESTING=False -DCMAKE_BUILD_TYPE=Release
    catkin_make install
    echo "source ~/catkin_ws/devel/setup.bash" >> ~/.bashrc
    source ~/.bashrc
  
### pointcloud 옵션을 사용하여 카메라 노드 실행
    roslaunch realsense2_camera rs_camera.launch filters:=pointcloud              // 뒤에 붙는 옵션은 다양하게 붙을 수 있다.
    rviz
 
위 명령어를 실행하고 Dispalys에서 [Add]를 눌러 PointCloud2, image, TF를 추가한다.
Global Options -> Fixed Frame -> camera_color_frame (축 자유롭게 선택가능)
PointCloud2 -> Topic -> /camera/depth/color/points
TF -> Frames -> camera_link, map 만 체크하고 나머지 체크 해제
image -> image Topic -> /camera/color/image_raw

### realsense camera를 이용한 SLAM
    sudo apt-get install ros-noetic-imu-filter-madgwick
    sudo apt-get install ros-noetic-rtabmap-ros
    sudo apt-get install ros-noetic-robot-localization
### SLAM 사용가능한 카메라 노드 실행
    roslaunch realsense2_camera opensource_tracking.launch
### SLAM 녹화하기
저장이 아니라 녹화 기능이므로 처음 카메라 노드를 실행시켰을때 아래 명령어를 입력해야 한다.
여러파일을 저장하려면 my_bagfile_1.bag <- 이름을 변경해야한다. 변경없이 실행시키면 데이터가 덮어씌어져 삭제된다.
                                
    rosbag record -O my_bagfile_1.bag /camera/aligned_depth_to_color/camera_info  camera/aligned_depth_to_color/image_raw /camera/color/camera_info /camera/color/image_raw /camera/imu /camera/imu_info /tf_static

    // 녹화한 파일 불러오기
    roscore >/dev/null 2>&1 &
    rosparam set use_sim_time true
    rosbag play my_bagfile_1.bag --clock                                  // 불러오고 싶은 파일 이름을 입력한다.
    roslaunch realsense2_camera opensource_tracking.launch offline:=true

    // 3D 맵 저장하기
    rosrun pcl_ros pointcloud_to_pcd input:=/rtabmap/cloud_map

    // 저장한 3D 맵 보기(뒤에 .pdc 파일은 본인에 맞게 변경해주어야 함)
    sudo apt-get install pcl-tools
    pcl_viewer 1543906154413083.pcd

    // 2D 맵 저장하기
    rosrun map_server map_saver map:=/rtabmap/proj_map –f my_map_1         // my_map_1 은 본인이 원하는 이름으로 저장 가능하다.
  
### 카메라 파라미터 값 변경하기
    rosrun rqt_reconfigure rqt_reconfigure

  
# NVIDIA Driver 설치 
    lshw -C display                                  // 본인 컴퓨터에 GPU가 물리적으로 설치 되었는지 확인
    ubuntu-drivers devices                           // 본인 컴퓨터에 설치 가능한 GPU 드라이버 찾기                           
    sudo apt install nvidia-driver-515               // 드라이버 뒤에 distro non-free 붙은거 아무거나 설치
    sudo reboot                                      // 재부팅 해야 드라이버 적용된다.
    nvidia-smi                                       // 설치 되었는지 확인한다.
  
# NVIDIA CUDA 설치
아래 링크에서 본인의 os, cpu, ubuntu 버전에 맞게 선택하여 설치 할것 / runfile(local) 설치해야한다.

https://developer.nvidia.com/cuda-toolkit-archive
    
    wget https://developer.download.nvidia.com/compute/cuda/11.7.0/local_installers/cuda_11.7.0_515.43.04_linux.run
    sudo sh cuda_11.7.0_515.43.04_linux.run
  
bashrc 파일을 수정해야 한다.
  
    gedit ~/.bashrc
  
아래의 문구를 추가해준다. 이때 /usr/local/cuda-버전숫자 이 경로로 들어가서 cuda 버전을 확인하여 그에 맞게 수정해준다.
  
    export PATH=/usr/local/cuda-11.7/bin${PATH:+:${PATH}}
    export LD_LIBRARY_PATH=/usr/local/cuda/lib64:${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
    source ~/.bashrc
    nvcc --version        // 설치 되었는지 확인
  
# cuDNN 설치
    
아래 링크에서 사용할 tensorflow와 cuda의 버전을 확인하여 호환되는 것을 다운 받는다. (너무 최신 버전이어도 안 좋을수 있다.)
  https://developer.nvidia.com/rdp/cudnn-archive

압축해제
    
    cd ~/Downloads
    tar xf cudnn-linux-x86_64-8.4.1.50_cuda11.6-archive.tar.xz                      // 본인이 설치한 압축파일의 이름을 복사, 붙여넣기 하여 압축 해제한다.

  여기서 압축해제한 폴더 이름은 cuda로 바꾼다.
  
    cd ~/Downloads
    sudo cp cuda/include/cudnn* /usr/local/cuda/include
    sudo cp cuda/lib/libcudnn* /usr/local/cuda/lib64
    sudo chmod a+r /usr/local/cuda/include/cudnn.h /usr/local/cuda/lib64/libcudnn*
    
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_adv_train.so.8.4.1 /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_adv_train.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8.4.1  /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8.4.1  /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8.4.1  /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_ops_train.so.8.4.1  /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_ops_train.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8.4.1 /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8
    sudo ln -sf /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn.so.8.4.1 /usr/local/cuda-11.7/targets/x86_64-linux/lib/libcudnn.so.8

  설치를 확인한다.
  
    ldconfig -p | grep cudnn                    // 아무것도 디스플레이 되지 않을때가 있다.
  
# tensorflow install
  
  아래는 tensorflow, cuda, cuDNN 호환성 판별 링크이며 버전 참고용으로만 사용한다.
  https://www.tensorflow.org/install/source?hl=ko#gpu

    cd
    python3 -V
    sudo apt install python3-venv
    mkdir my_tensorflow
    cd my_tensorflow                                // Python 3 가상 환경을 저장할 디렉토리로 이동합니다.
    python3 -m venv venv                            // 가상 환경을 생성
    source venv/bin/activate                        // 활성화되면 $PATH 변수의 시작 부분에 가상 환경의 bin 디렉토리가 추가됩니다. 현재 사용 중인 가상 환경의 이름이 표시됩니다. 이 경우에는 venv입니다.
    pip install --upgrade pip
    pip install --upgrade tensorflow
    python -c 'import tensorflow as tf; print(tf.__version__)'              // tensorflow 버전 확인
  
작업이 완료되면 비활성화를 입력하여 환경을 비활성화하면 정상 셸로 돌아갑니다.
  
    deactivate

# realsense object detection

    cd
    git clone https://github.com/WongKinYiu/yolov7.git
    sudo apt install python3-pip
    pip3 install -U PyYAML
    pip3 install tqdm
    pip3 install cython
    pip3 install numpy                                              //==1.19.5   //pip3 install numpy==1.20.3
    sudo apt install libjpeg-dev
    pip3 install matplotlib
    sudo apt-get install libhdf5-serial-dev hdf5-tools libhdf5-dev zip libjpeg8-dev libblas-dev
    sudo apt install gfortran
    sudo apt install libopenblas-dev
    sudo apt install liblapack-dev
    pip3 install scipy
    pip3 install typing-extensions
    pip3 install torch
    pip3 install torchvision
    sudo apt-get install python3-opencv
    wget https://nvidia.box.com/shared/static/p57jwntv436lfrd78inwl7iml6p13fzh.whl -O torch-1.8.0-cp36-cp36m-linux_aarch64.whl   // 생략 가능할듯
    sudo apt-get install python3-pip libopenblas-base libopenmpi-dev libomp-dev
    pip3 install torch-1.8.0-cp36-cp36m-linux_aarch64.whl
    sudo apt-get install libjpeg-dev zlib1g-dev libpython3-dev libavcodec-dev libavformat-dev libswscale-dev
    git clone --branch v0.9.0 https://github.com/pytorch/vision torchvision
    cd torchvision
    export BUILD_VERSION=0.9.0
    python3 setup.py install --user
    cd
    pip3 install seaborn
    cd yolov7
    wget "https://github.com/WongKinYiu/yolov7/releases/download/v0.1/yolov7-tiny.pt" 
    python3 detect.py --weights yolov7-tiny.pt --source inference/images/horses.jpg

이제 /home/yolov7/runs/detect/exp/horses.jpg 경로의 말 사진에서 주위에 네모박스가 있는것을 확인할 수 있다.
  
    cd
    git clone https://github.com/jetsonhacks/installRealSenseSDK.git
    cd installRealSenseSDK
  
/home/installRealSenseSDK/buildLibrealsense.sh 경로의 파일에 들어가서 153줄에 있는 time make -j$NUM_PROCS  -->  time make -j1 라고 바꿔준다

다음으로 파일을 설치한다.
  
    ./buildLibrealsense.sh
  
이제 ~/yolov7/detect.py 파일 내용을 아래로 바꾼다. ( ros가 포함된 코드는 깃허브의 detect.py 로 작성 )
  
  ```py
import argparse
import time
from pathlib import Path

import cv2
import torch
import torch.backends.cudnn as cudnn
from numpy import random

from models.experimental import attempt_load
from utils.datasets import LoadStreams, LoadImages
from utils.general import check_img_size, check_requirements, check_imshow, non_max_suppression, apply_classifier, \
    scale_coords, xyxy2xywh, strip_optimizer, set_logging, increment_path
from utils.plots import plot_one_box
from utils.torch_utils import select_device, load_classifier, time_synchronized, TracedModel

import pyrealsense2 as rs
import numpy as np

def detect(save_img=False):
    source, weights, view_img, save_txt, imgsz, trace = opt.source, opt.weights, opt.view_img, opt.save_txt, opt.img_size, not opt.no_trace
    save_img = not opt.nosave and not source.endswith('.txt')  # save inference images
    webcam = source.isnumeric() or source.endswith('.txt') or source.lower().startswith(
        ('rtsp://', 'rtmp://', 'http://', 'https://'))

    # Directories
    save_dir = Path(increment_path(Path(opt.project) / opt.name, exist_ok=opt.exist_ok))  # increment run
    (save_dir / 'labels' if save_txt else save_dir).mkdir(parents=True, exist_ok=True)  # make dir

    # Initialize
    set_logging()
    device = select_device(opt.device)
    half = device.type != 'cpu'  # half precision only supported on CUDA

    # Load model
    model = attempt_load(weights, map_location=device)  # load FP32 model
    stride = int(model.stride.max())  # model stride
    imgsz = check_img_size(imgsz, s=stride)  # check img_size

    if trace:
        model = TracedModel(model, device, opt.img_size)

    if half:
        model.half()  # to FP16

    # Second-stage classifier
    classify = False
    if classify:
        modelc = load_classifier(name='resnet101', n=2)  # initialize
        modelc.load_state_dict(torch.load('weights/resnet101.pt', map_location=device)['model']).to(device).eval()

    # Set Dataloader
    vid_path, vid_writer = None, None
#    if webcam:
#        view_img = check_imshow()
#        cudnn.benchmark = True  # set True to speed up constant image size inference
#        dataset = LoadStreams(source, img_size=imgsz, stride=stride)
#    else:
#        dataset = LoadImages(source, img_size=imgsz, stride=stride)

    # Get names and colors
    names = model.module.names if hasattr(model, 'module') else model.names
    colors = [[random.randint(0, 255) for _ in range(3)] for _ in names]

    # Run inference
    if device.type != 'cpu':
        model(torch.zeros(1, 3, imgsz, imgsz).to(device).type_as(next(model.parameters())))  # run once
    old_img_w = old_img_h = imgsz
    old_img_b = 1

    config = rs.config()
    config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)
    config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)

    pipeline = rs.pipeline()
    profile = pipeline.start(config)

    align_to = rs.stream.color
    align = rs.align(align_to)

    while(True):
       frames = pipeline.wait_for_frames()

       aligned_frames = align.process(frames)
       color_frame = aligned_frames.get_color_frame()
       depth_frame = aligned_frames.get_depth_frame()
       if not depth_frame or not color_frame:
           continue

       img = np.asanyarray(color_frame.get_data())
       depth_image = np.asanyarray(depth_frame.get_data())
       depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.08), cv2.COLORMAP_JET)

       #Letterbox
       im0 = img.copy()
       img = img[np.newaxis, :, :, :]

       #stack
       img = np.stack(img, 0)

       #convert
       img = img[..., ::-1].transpose((0, 3, 1, 2)) #BGR to RGB, BHWC to BCHW
       img = np.ascontiguousarray(img)

       img = torch.from_numpy(img).to(device)
       img = img.half() if half else img.float()  # uint8 to fp16/32
       img /= 255.0  # 0 - 255 to 0.0 - 1.0
       if img.ndimension() == 3:
           img = img.unsqueeze(0)

        # Warmup
       if device.type != 'cpu' and (old_img_b != img.shape[0] or old_img_h != img.shape[2] or old_img_w != img.shape[3]):
           old_img_b = img.shape[0]
           old_img_h = img.shape[2]
           old_img_w = img.shape[3]
           for i in range(3):
               model(img, augment=opt.augment)[0]

        # Inference
       t1 = time_synchronized()
       with torch.no_grad():   # Calculating gradients would cause a GPU memory leak
           pred = model(img, augment=opt.augment)[0]
       t2 = time_synchronized()

       # Apply NMS
       pred = non_max_suppression(pred, opt.conf_thres, opt.iou_thres, classes=opt.classes, agnostic=opt.agnostic_nms)
       t3 = time_synchronized()

       # Apply Classifier
       # if classify:
       #    pred = apply_classifier(pred, modelc, img, im0s)

       # Process detections
       for i, det in enumerate(pred):  # detections per image

           gn = torch.tensor(im0.shape)[[1, 0, 1, 0]]  # normalization gain whwh
           if len(det):
               # Rescale boxes from img_size to im0 size
               det[:, :4] = scale_coords(img.shape[2:], det[:, :4], im0.shape).round()

               # Print results
               for c in det[:, -1].unique():
                   n = (det[:, -1] == c).sum()  # detections per class
                   #s += f"{n} {names[int(c)]}{'s' * (n > 1)}, "  # add to string

               # Write results
               for *xyxy, conf, cls in reversed(det):
                   c = int(cls) #integer class
                   label = f'{names[c]} {conf:.2f}'
                   plot_one_box(xyxy, im0, label=label, color=colors[int(cls)], line_thickness=2)
                   plot_one_box(xyxy, depth_colormap, label=label, color=colors[int(cls)], line_thickness=2)

           # Print time (inference + NMS)
           #print(f'{s}Done. ({(1E3 * (t2 - t1)):.1f}ms) Inference, ({(1E3 * (t3 - t2)):.1f}ms) NMS')

           # Stream results
               cv2.imshow("Recongnition result", im0)
               cv2.imshow("Recongnition result depth", depth_colormap)
               if cv2.waitKey(1) & 0xFF == ord('q'):
                   break

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--weights', nargs='+', type=str, default='yolov7.pt', help='model.pt path(s)')
    parser.add_argument('--source', type=str, default='inference/images', help='source')  # file/folder, 0 for webcam
    parser.add_argument('--img-size', type=int, default=640, help='inference size (pixels)')
    parser.add_argument('--conf-thres', type=float, default=0.25, help='object confidence threshold')
    parser.add_argument('--iou-thres', type=float, default=0.45, help='IOU threshold for NMS')
    parser.add_argument('--device', default='', help='cuda device, i.e. 0 or 0,1,2,3 or cpu')
    parser.add_argument('--view-img', action='store_true', help='display results')
    parser.add_argument('--save-txt', action='store_true', help='save results to *.txt')
    parser.add_argument('--save-conf', action='store_true', help='save confidences in --save-txt labels')
    parser.add_argument('--nosave', action='store_true', help='do not save images/videos')
    parser.add_argument('--classes', nargs='+', type=int, help='filter by class: --class 0, or --class 0 2 3')
    parser.add_argument('--agnostic-nms', action='store_true', help='class-agnostic NMS')
    parser.add_argument('--augment', action='store_true', help='augmented inference')
    parser.add_argument('--update', action='store_true', help='update all models')
    parser.add_argument('--project', default='runs/detect', help='save results to project/name')
    parser.add_argument('--name', default='exp', help='save results to project/name')
    parser.add_argument('--exist-ok', action='store_true', help='existing project/name ok, do not increment')
    parser.add_argument('--no-trace', action='store_true', help='don`t trace model')
    opt = parser.parse_args()
    print(opt)
    #check_requirements(exclude=('pycocotools', 'thop'))

    with torch.no_grad():
        if opt.update:  # update all models (to fix SourceChangeWarning)
            for opt.weights in ['yolov7.pt']:
                detect()
                strip_optimizer(opt.weights)
        else:
            detect()
```

## realsense D455 사물인식 실행
  
    cd ~/yolov7
    python3 detect.py

### 자율주행을 위한 ros 의존성 패키지 설치 (생략가능)

    sudo apt-get install ros-melodic-joy ros-melodic-teleop-twist-joy \
    ros-melodic-teleop-twist-keyboard ros-melodic-laser-proc \
    ros-melodic-rgbd-launch ros-melodic-depthimage-to-laserscan \
    ros-melodic-rosserial-arduino ros-melodic-rosserial-python \
    ros-melodic-rosserial-server ros-melodic-rosserial-client \
    ros-melodic-rosserial-msgs ros-melodic-amcl ros-melodic-map-server \
    ros-melodic-move-base ros-melodic-urdf ros-melodic-xacro \
    ros-melodic-compressed-image-transport ros-melodic-rqt* \
    ros-melodic-gmapping ros-melodic-navigation ros-melodic-interactive-markers
    
### turtlebot3 패키지 설치 (생략가능)

    cd ~/catkin_ws/src/
    git clone -b melodic-devel https://github.com/ROBOTIS-GIT/DynamixelSDK.git
    git clone -b melodic-devel https://github.com/ROBOTIS-GIT/turtlebot3_msgs.git
    git clone -b melodic-devel https://github.com/ROBOTIS-GIT/turtlebot3.git
    cd ~/catkin_ws
    catkin_make
    source devel/setup.bash 
    echo "export TURTLEBOT3_MODEL=burger" >> ~/.bashrc
    
### cartographer 패키지 설치 (생략가능)
    
    sudo apt-get update
    sudo apt-get install -y python-wstool python-rosdep ninja-build stow
    cd ~/catkin_ws
    wstool init src
    wstool merge -t src https://raw.githubusercontent.com/cartographer-project/cartographer_ros/master/cartographer_ros.rosinstall
    wstool update -t src
    sudo rosdep init
    rosdep update
    rosdep install -y --from-paths src --ignore-src -r
    src/cartographer/scripts/install_abseil.sh
    catkin_make_isolated --install --use-ninja
    source ~/catkin_ws/install_isolated/setup.bash                                // 매번 터미널에 입력하기 귀찮으면 ~/.bashrc 에 작성
    roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=cartographer   // cartographer slam 실행
    
catkin_make_isolated  이 빌드 명령어는 cartographer 패키지를 빌드하는데 필수적이며 이 과정을 통해 빌드된 실행 파일은 ~/catkin_ws/install_isolated/share 안에 위치하게 된다.
하지만 이 때문에 기존에 src에 있던 다른 여러가지 패키지들이 빌드되지 않을 수 있다.
이는 ~/catkin_ws/install_isolated/share 경로 안에 빌드를 원하는 패키지를 삭제하고 하나의 패키지만 따로 빌드하는 아래의 명령어를 통해 빌드 할 수 있다.

    catkin_make --only-pkg-with-deps [패키지 명]
    
### turtlebot3_slam 및 navigation 을 정상적으로 실행시키기 위한 bringup 패키지 생성 (생략가능)

rplidar, arduino를 실행시키고 tf frame 변환관계 , /odom, /imu, /joint_states 토픽을 publish 하는 패키지를 제작하여 사용하였다.
이를 통해 pc, lidar, arduino 만을 사용해서 turtlebot3 패키지를 사용할 수 있으며 navigation 코드 또한 사용 할 수 있다.

    roslaunch henes_bringup henes_bringup.launch

# iAHRS 설치법

    ls -l /dev |grep ttyUSB                                 // imu 센서의 포트를 찾는다. ex) /dev/ttyUSB1
    cd catkin_ws/src
    git clone https://github.com/wookbin/iahrs_driver.git
    sudo usermod -a -G dialout $USER
    lsusb
    udevadm info -a /dev/ttyUSB1 | grep '{serial}'

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/1bb98caa-c175-4c9a-b77d-d72e38d66271)

    cd /etc/udev/rules.d
    nano IMU.rules

아래 내용 입력

    KERNEL=="ttyUSB1", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="0001", MODE:="0666", GROUP:="dialout", SYMLINK+="IMU" 

아래 명령어 입력후 pc 재시작

    sudo service udev restart
    cd ~/catkin_ws/src/iahrs_driver/src
    gedit iahrs_driver.cpp

아래 코드 변경

    [변경 전] #define SERIAL_PORT        "/dev/IMU" 
    [변경 후] #define SERIAL_PORT        "/dev/ttyUSB1" 

    cd ~/catkin_ws
    catkin_make

imu 실행

    roscore
    roslaunch iahrs_driver iahrs_driver.launch

# GPS 설치법

준비

    1. 국토지리정보원 회원가입
    2. 국토정보플랫폼 화면 이동
    3. 공간정보 -> 위성 기준점 서비스 -> 네트워크 RTK 서비스 클릭
    4. VRS ID 에서 아이디 만든 후 등록(패스워드는 ngii 고정)
    5. GPS HW 모듈연결(ZED-F9P를 C타입-USB PC와 연결)

    ls -l /dev/ttyACM*                // 우분투 터미널에서 GPS RTK 보드 인식 확인
    sudo chmod 777 /dev/ttyACM0       // 대부분은 ttyACM0로 인식하나 본인의 경우에 맞게 수정할 것

## RTK-GPS install

    cd ~/catkin_ws/src
    git clone https://github.com/DigSafeMQP2020/ublox_f9p.git
    cd ublox_f9p
    git clone https://github.com/tilk/rtcm_msgs.git
    git clone https://github.com/ros-agriculture/ntrip_ros.git
    cd ~/catkin_ws
    catkin_make

## 파일 수정

~catkin_ws/src/ublox_f9p/ublox_gps/launch/ublox_device.launch 파일의 일부를 수정한다

    <?xml version="1.0" encoding="UTF-8"?>

    <launch>
      <arg name="node_name" value="ublox_gps"/>
      <arg name="param_file_name" value="zed-f9p" />
      <arg name="output" default="screen" />
      <arg name="respawn" default="true" />
      <arg name="respawn_delay" default="30" />
      <arg name="clear_params" default="true" />

      <node pkg="ublox_gps" type="ublox_gps" name="$(arg node_name)" 
            output="$(arg output)" 
            clear_params="$(arg clear_params)" 
            respawn="$(arg respawn)" 
            respawn_delay="$(arg respawn_delay)">
        <rosparam command="load" 
                  file="$(find ublox_gps)/config/zed-f9p.yaml" />
      </node>
    </launch>

다음으로
/catkin_ws/src/ublox_f9p/ublox_gps/config/zed_f9p.yaml 파일에서 device가 본인이 연결한 포트와 일치하는지 확인할 것 ex) device: /dev/ttyACM1

다음으로 ~/catkin_ws/src/ublox_f9p/ublox_gps/src/node.cpp 파일에서 1861번줄 문장을 수정할것

     //param_nh.param("rtcm_topic", rtcm_topic, std::string("rtcm"));              // 원래 코드
      param_nh.param("/ublox_gps/rtcm", rtcm_topic, std::string("rtcm"));          // 변경한 코드

~/catkin_ws/src/ublox_f9p/ntrip_ros/launch/ntrip_ros.launch 파일 아래와 같이 수정할것

<?xml version="1.0" encoding="UTF-8"?>

    <launch>
      <include file="$(find ublox_gps)/launch/ublox_device.launch"/>
      <node pkg="ntrip_ros" type="ntripclient.py" name="ntrip_ros" output="screen">
          <param name="rtcm_topic" value="/ublox_gps/rtcm"/>
          <param name="ntrip_server" value="RTS1.ngii.go.kr:2101"/>
          <param name="ntrip_user" value="pace0807"/>
          <param name="ntrip_pass" value="ngii"/>
          <param name="ntrip_stream" value="VRS-RTCM31"/>
          <param name="nmea_gga" value="$GPGGA,123710.969,3646.149,N,12656.085,E,1,12,1.0,0.0,M,0.0,M,,*6E"/>
      </node>
    </launch>

## GPS 실행

    cd ~/catkin_ws
    catkin_make
    source devel/setup.bash
    roscore
    roslaunch ublox_gps ublox_device.launch           // source devel/setup.bash   각각의 터미널에서 입력해 줘야한다.
    roslaunch ntrip_ros ntrip_ros.launch              // source devel/setup.bash

## GPS-UTM-LLA

    sudo apt-get install ros-melodic-geographic-*
    sudo apt-get install geographiclib-*
    sudo apt-get install libgeographiclib-*
    sudo apt-get install libgeographic-dev
    sudo ln -s /usr/share/cmake/geographiclib/FindGeographicLib.cmake /usr/share/cmake-3.10/Modules/

    cd catkin_ws/src 
    git clone https://github.com/arpg/ROS-UTM-LLA.git
    cd ..
    catkin_make

## geonav_transform

    cd catkin_ws/src
    git clone https://github.com/bsb808/geonav_transform.git
    cd ..
    catkin_make

## DigSafeMQP2020/ublox_f9p 파일 말고 ros-agriculture/ublox_f9p 파일을 사용할때 빌드 안돼는 문제 해결법

~catkin_ws/src/ublox_f9p/ublox_gps/CMakeLists.txt 파일의 26줄의 코드를 수정한다.

    //SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")        기존거
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")       // 수정한거

# ubuntu에서 Arduino 설치법

## 1. Install package

    $ sudo apt-get install ros-melodic-rosserial-arduino              // arduino 시리얼 통신을 위한 패키지 설치
    $ sudo apt-get install ros-melodic-rosserial                      // PC 시리얼 통신을 위한 패키지 설치

## 2. InstalArduino IDE

https://www.arduino.cc/en/Main/Software 해당 링크로 들어가서 아래로 스크롤 하여 Previous Release 1.8.18을 클릭한다.(다른 방법을 쓰던 release 파일로 들어가면 된다.)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/a00151d1-182d-4b40-a705-ee70414f888a)
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/3ec72ed3-cf91-4a0b-99ce-0f489e1bb6ec)

원하는 아두이노 버전을(우리는 1.8.9버전 사용함) 본인 컴퓨터에 맞게 선택하는데, 마우스 우클릭으로 링크주소복사를 한다.

    $ cd ~/Downloads                                                                 // 터미널을 통해 Downloads 경로로 이동하여 그 위치에 아두이노 ide를 설치 할 것이다.
    $ sudo wget https://downloads.arduino.cc/arduino-1.8.13-linux64.tar.xz           // 직접 "sudo wget " 을 타이핑 하고 복사한 링크를 붙여넣기 한다. (Linux 64bit의 경우)
    $ sudo wget https://downloads.arduino.cc/arduino-1.8.19-linuxaarch64.tar.xz      // 본인 cpu 상황에 따라 둘중 하나만 선택하여 터미널에 입력하면 된다.(Linux ARM64의 경우) 

여기서 설치한 아두이노 압축파일이 Downloads 경로에 위치하지 않다면 그냥 마우스로 끌어다가 Downloads에 옮겨놓는다.

    $ cd ~/Downloads                                       // Downloads로 이동하는 것인데, 원래 이 경로에 있었다면 상관없다.
    $ tar xf arduino-1.8.9.tar.xz                             // 해당 파일 압축 풀기 ( [filename] 을 지우고 arduino-1.8.9-linux64 이런 식으로 써있는거 복붙해준다.)
    $ mv -f arduino-1.8.9 ~/                            // 홈 경로로 폴더 이동  (압축을 풀면 arduino-1.8.9 같은 이름의 파일이 생성되었을 것이다. 이를 홈 경로로 옮긴다.)
    $ cd ~/arduino-1.8.9                                // [arduino_forder] 지우고 arduino-1.8.9 같은거 복붙, 위의 mv 명령의 경우도 마찬가지 였다.
    $ sudo ./install.sh                                    // 아두이노 설치함

## 4. Run Arduino

    $ arduino  // 아두이노 실햄

board 와 port 설정 (사진 내용은 그냥 예시임, 그리고 물리적으로 pc와 아두이노 연결되야함)
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/e956b82c-404e-4140-b945-5f1450123fe4)

## 포트 찾기

    ls -l /dev |grep ttyUSB
    또는
    dmesg | grep tty

## Arduino 직렬 통신 시작

    rosrun rosserial_python serial_node.py _port:=/dev/ttyACM0

# xbox controller 세팅법

https://makingrobot.tistory.com/38

    sudo apt-get update
    sudo apt-add-repository ppa:rael-gc/ubuntu-xboxdrv
    sudo apt-get install ubuntu-xboxdrv

    sudo systemctl enable xboxdrv.service
    sudo apt install sysfsutils

    sudo nano /etc/sysfs.conf

아래 내용 입력

    /module/bluetooth/parameters/disable_ertm=1

블루투스로 xbox controller 를 연결하여 connect 되는지 확인후, pc 재부팅 잘 동작하는지 확인

    cd /dev/input
    sudo jstest /dev/input/js4

    sudo chmod a+rw /dev/input/js4
    cd ~/catkin_ws/src
    git clone https://github.com/ros-teleop/teleop_twist_joy.git
    git clone https://github.com/ros-drivers/joystick_drivers.git
    sudo apt-get install libspnav-dev
    cd ~/catkin_ws
    catkin_make

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/64a78176-d50d-4571-912e-c995555bb46d)
