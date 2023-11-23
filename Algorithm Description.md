# 1. 실외주행 알고리즘 (waypoints_navigation_node.cpp)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/70ebd034-203c-45b7-aac0-285fc1b48c68)

    실외주행에서 로봇의 현재위치는 GPS 로 받고있으며, 로봇이 센서가 구동된 초기 상태에서 좌, 우로 얼마나 조향했는지 판별하는 것은 IMU로 부터 센싱하고 있다.

    주행 목적지가 정해지면, 주행로봇이 어떠한 경로로 이동할지 미리 경유점을 설정하였다.
    0번 경유점을 가장 먼저 지나야 하므로 로봇은 0번 경유점을 현재의 wp_go_id 를 0으로 설정한다. 로봇이 해당 경유점에 도착하면 다음 경유점인 1번을 현재의 wp_go_id = 1로 설정한다.

    우선 가야할 경유점이 wp_go_id=0일때 로봇의 현재 위치에서 [0]번 경유점으로 얼마나 조향을 틀어야 하는지는 상대좌표 계산을 통해 이루어진다.

  ![image](https://github.com/kimjunho654/henes_develop/assets/105560901/a2d0797c-4b66-4f69-9cb9-ca51ca2cea38)
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/f6a921d7-3a22-4ff9-bc48-07faf483f42c)

    상대좌표 계산은 다음과 같이 이루어 진다.

    *목표위치 : 미리 정의된 경유점 좌표,  현재위치 : GPS UTM 변환 값
    1. 상대 좌표(= 목표위치 - 현재위치)를 계산한다. 이는 x, y 값에 대해 계산된다.

    2. 위 좌표에는 로봇이 현재 어느 방향을 바라보고 있는지에 대해 전혀 고려되지 않는다.
       로봇의 회전을 IMU로 부터 센싱하여, 회전행렬을 적용하여 로봇의 방향도 고려한 상대좌표를 계산한다.

    3. 방향도 고려한 상대좌표 x, y를 통해 조향각도를 계산한다.

## gps로 부터 현재위치 얻기

```cpp
void utm_Callback(const geometry_msgs::PoseStamped::ConstPtr& msgs)
{
        utm_fix.header = msgs->header;
        utm_fix.pose.position.x     =  msgs->pose.position.x ;
        utm_fix.pose.position.y     =  msgs->pose.position.y ;

        my_pose.x     =  utm_fix.pose.position.x;      //UTM 좌표의 경우 X,Y 좌표가 90도 회전되어 있음
        my_pose.y     =  utm_fix.pose.position.y;      //UTM 좌표의 경우 X,Y 좌표가 90도 회전되어 있음
}

ros::Subscriber utm_sub                  = n.subscribe("/utm",1,&utm_Callback);
```

    이때 my_pose.x는 GPS로 /ublox_gps/fix를 센싱하고 이를 /utm으로 변환한 값을 사용하며 로봇의 현재위치를 의미한다. 
    또한 센싱값을 칼만필터를 적용하여 /ublox_gps/fix --> /filtered_gps_topic --> /utm 이런식으로 변환하여 사용하였다.

## IMU 값 얻기
```cpp
void imu_Callback(const sensor_msgs::Imu::ConstPtr& msg)
{
        tf2::Quaternion q(
        msg->orientation.x,
        msg->orientation.y,
        msg->orientation.z,
        msg->orientation.w);

        tf2::Matrix3x3 m(q);

        m.getRPY(roll, pitch, yaw);

        yaw = yaw+ DEG2RAD(imu_offset);
        my_pose.theta = yaw;
        //printf("%6.3lf(rad)  %6.3lf \n",yaw, yaw*180/3.14159);
}

ros::Subscriber imu_sub                  = n.subscribe("/imu/data",10,&imu_Callback);
```

    IMU 데이터로부터 x, y, z축 회전을 나타내는 값을 얻어내고 이 중에서 차량의 조향에 관련한 회전인 yaw값을 얻어온다.
    상대좌표 계산에 있어서 로봇의 회전을 고려해야 하는데 yaw를 my_pose.theta 값으로 설정한다.

    이때 yaw 값에서 offset을 설정하는 이유는 위 그림처럼 상대좌표 계산 과정 설명에 있어서 로봇이 0도 인, 오른쪽을 바라보고 있다고 가정하고 설명했는데
    사실 로봇은 처음 시작상태에서 다른 방향을 바라보고 시작할 수도 있다. 이에 의해서 방향의 초기상태인 offset 값을 개발자가 스스로 알아내서 반영해 주어야 한다. float imu_offset = -53;

## 경유점 지정하기
```cpp
void init_waypoint(void)
{
        my_waypoints_list[0].x = 315719.89;
        my_waypoints_list[0].y = 4071242.41;

        my_waypoints_list[1].x = 315723.16;
        my_waypoints_list[1].y = 4071239.60;

        my_waypoints_list[2].x = 315727.92;
        my_waypoints_list[2].y = 4071232.56;
        .
        .
        .
        my_waypoints_list[12].x = 315721.12;
        my_waypoints_list[12].y = 4071244.21;

        set_delivery_id = 6;
        wp_finish_id = 12;
}
```
    여기서 목적지마다 각각 설정된 set_delivery_id 값에 도착했을 경우 로봇은 정지하여, 손님이 배달 물품을 수령하는 과정을 수행한다.
    이때 wp_go_id는 증가하지 않다가 손님이 배달을 완료했다는 topic을 받으면 wp_go_id는 증가하며 다음 경유지로 주행을 시작한다.

    wp_finish_id 에 도착하면 배달과정이 모두 완료되어 정지한다. 이때 노드가 종료되는 것이 아니라 새로운 목적지를 sub 받고, 주행 시작을 sub 받으면 다시 새롭게 실외주행 알고리즘을 수행한다.

## 상대좌표 계산하기
```cpp
void base_link_tf_utm(void)
{
        double waypoint_pos_base_link_x     = 0.0;
        double waypoint_pos_base_link_y     = 0.0;
        double waypoint_pos_base_link_theta = 0.0;
        double tf_base_map_x,tf_base_map_y;
        double waypoint_angle, waypoint_distance;
        ///////////////////////////////////////////////////
        tf_base_map_x = -my_pose.x;   //상대좌표로 변환  no translation
        tf_base_map_y = -my_pose.y;

        tf_base_map_x += my_target_pose_goal.x;   //상대좌표로 변환  no translation
        tf_base_map_y += my_target_pose_goal.y;

        waypoint_pos_base_link_x = tf_base_map_x * cos(my_pose.theta)  + tf_base_map_y * sin(my_pose.theta);   // rotation_matrix
        waypoint_pos_base_link_y = -tf_base_map_x * sin(my_pose.theta) + tf_base_map_y * cos(my_pose.theta);

        waypoint_angle = atan2(waypoint_pos_base_link_y ,waypoint_pos_base_link_x);
        waypoint_distance = sqrt(waypoint_pos_base_link_x*waypoint_pos_base_link_x  + waypoint_pos_base_link_y*waypoint_pos_base_link_y);
        car_angle = RAD2DEG(waypoint_angle);

        ROS_INFO(" X : %6.3lf   Y : %6.3lf  Yaw : %6.3lf ", my_pose.x, my_pose.y, RAD2DEG(my_pose.theta));
        ROS_INFO(" b_x : %6.3lf  b_y : %6.3lf",waypoint_pos_base_link_x,waypoint_pos_base_link_y);

}
```
    my_target_pose_goal.x는 다음 경유점의 좌표이며 목적지에 따라 각각의 함수에서 미리 정의된 경유점 좌표로 설정된다. 그것이 my_waypoints_list[wp_go_id].x; 이며 여기서 wp_go_id 인덱스는 0부터 시작하여, 각 경유점에 도착하면 1씩 증가하여 다음 경유점으로 주행하도록 한다.

    목표 경유점 위치와 현재위치를 통해 방향이 고려되지 않은 tf_base_map_x, y 를 계산한다.

    그리고 my_pose.theta 을 통해 방향을 고려한 상대좌표인 waypoint_pos_base_link_x, y를 계산한다.

    현재 위치에서 다음 경유점을 가기 위해 로봇이 조향해야 하는 각도인 waypoint_angle을 계산한다. 이를 degree 값으로 바꾸어 car_angle 에 저장한다.

## 목적지 설정
```cpp
string target_string;
bool sub_destination = false;

void destination_Callback(const std_msgs::String::ConstPtr& msg)
{
    if(msg->data == "Unitophia" || (msg->data == "Multi_Media_Gwan" || msg->data == "Hak_Ye_Gwan" || msg->data == "BRIX_Gwan" || msg->data == "San_Hak_Hyeop_Ryeok_Gwan" || msg->data == "Gong_Hak_Gwan" || msg->data == "Library" || msg->data == "Antire_preneur_Gwan" || msg->data == "Naturel_Science_Gwan" || msg->data == "Humanities_Social_Science_Gwan" || msg->data == "Main_University" || msg->data == "Global_Village" || msg->data == "Hyang_333" || msg->data == "test") ){

        if(start_command == false) {
            sub_destination = true;
            target_string = msg->data;
        }
    }
}

ros::Subscriber destination_sub  = n.subscribe<std_msgs::String>("/destination",10,&destination_Callback);
```
    목적지를 sub받아 target_string 에 저장한다. 이때 한번 sub 받은 목적지가 주행 도중에 변경되는 것을 막기 위해 sub_destination을 통해 방지한다.

## 주행 도중 목적지 변경을 방지
```cpp
    while (ros::ok() && (sub_destination == false) ){

        ROS_INFO("Wati sub Destination");

        ros::spinOnce();
    }
```

## 경로지정
```cpp
    if(target_string == "init"){ init_waypoint(); } ////////////////////////////////////////////////////////////////////////////////
    else if( target_string == "Unitophia" ){ Unitophia();  destination_data = "Unitophia"; }
    else if( target_string == "Multi_Media_Gwan" ){ Multi_Media_Gwan();  destination_data = "Multi_Media_Gwan"; }
    else if( target_string == "Hak_Ye_Gwan" ){ Hak_Ye_Gwan();  destination_data = "Hak_Ye_Gwan"; }
    else if( target_string == "BRIX_Gwan" ){ BRIX_Gwan();  destination_data = "BRIX_Gwan"; }
    else if( target_string == "San_Hak_Hyeop_Ryeok_Gwan" ){ San_Hak_Hyeop_Ryeok_Gwan();  destination_data = "San_Hak_Hyeop_Ryeok_Gwan"; }
    else if( target_string == "Gong_Hak_Gwan" ){ Gong_Hak_Gwan();  destination_data = "Gong_Hak_Gwan"; }
    else if( target_string == "Library" ){ Library();  destination_data = "Library"; }
    else if( target_string == "Antire_preneur_Gwan" ){ Antire_preneur_Gwan();  destination_data = "Antire_preneur_Gwan"; }
    else if( target_string == "Naturel_Science_Gwan" ){ Naturel_Science_Gwan();  destination_data = "Naturel_Science_Gwan"; }
    else if( target_string == "Humanities_Social_Science_Gwan" ){ Humanities_Social_Science_Gwan();  destination_data = "Humanities_Social_Science_Gwan"; }
    else if( target_string == "Main_University" ){ Main_University();  destination_data = "Main_University"; }
    else if( target_string == "Global_Village" ){ Global_Village();  destination_data = "Global_Village"; }
    else if( target_string == "Hyang_333" ){ Hyang_333();   destination_data = "Hyang_333"; }
    else if( target_string == "test" ) { test_multi_hakyeah();  destination_data = "test_multi_hakyeah"; }
```
    sub 받은 목적지에 따라 각각 다른 경로 설정 함수가 실행된다.

## 출발 명령 sub 후, 주행 시작
```cpp
void start_command_Callback(const std_msgs::Bool::ConstPtr& msg)
{
    start_command = msg->data;
}

ros::Subscriber start_command_sub      = n.subscribe("/start_command",10,&start_command_Callback);
```
```cpp    while (ros::ok()) {

        if(emergency_stop == false){
            if(start_command == true){
                c_speed.data = 230;
                s_angle.data = car_angle;
            }
            else if(start_command == false){
                c_speed.data = 0;
                s_angle.data = 0;
            }
        }
        else if(emergency_stop == true){
            c_speed.data = 0;
            s_angle.data = 0;
        }
```
    출발 명령이 true 일 경우, 주행 명령값 = 230을 pub한다. 이는 속도 or pwm 값을 230으로 준다는 의미는 아니다.

    이 주행 명령값은 speed_pid_control 노드에서 sub 받은 후, 실제 차량 속도제어를 하는데 사용되는 값이며 단순히 230 이면 전진, 0이면 정지 하라는 명령으로 이해하면 된다.

## 경유점, 손님에게 도달했는지 판별
```cpp
            if( (pos_error_x <= WayPoint_X_Tor) && (pos_error_y <= WayPoint_Y_Tor )) {
                printf("----------------------------\n");
                printf("Arrvied at My WayPoint[%3d] !\n",wp_go_id);
                printf("----------------------------\n");

                if(wp_go_id != set_delivery_id) { wp_go_id++; }

                if( (wp_go_id == set_delivery_id) && (received_product == false) ) {
                    c_speed.data = 0;
                    s_angle.data = 0;
            }
                else if( (wp_go_id == set_delivery_id) && (received_product == true) ) {
                    c_speed.data = 230;
                    s_angle.data = car_angle;
                    wp_go_id++;
                    received_product = false;
                }

            }
```
    pos_error_x 와 pos_error_y 값이 일정 값 이하일 경우, 즉 현재의 위치가 각 경유점의 위치에 일정 범위 안에 도착했을경우 현재 목표 위치에 도착했음을 판단한다.

    wp_go_id 와 set_delivery_id를 비교하여 서로 다르다면 로봇이 손님에게 도착하지 않았다는 뜻이므로 wp_go_id를 증가시키고 다음 경유점을 향해 이동한다.

    만약 wp_go_id 와 set_delivery_id이 같다면, 로봇이 손님에게 도착했으므로 정지한다. 이는 상품이 수령되지 않았다면 계속 정지한다. received_product = false
    손님이 상품을 수령했을 경우(received_product = true) 로봇은 wp_go_id를 증가시켜 주행을 시작하여 다시 시작위치로 왕복해서 돌아간다.

## 최종 목적지 도착 판별 / 대기 후 다음 배달 시작
```cpp
            if(wp_go_id > wp_finish_id) {
                start_command = false;
                c_speed.data = 0;
                s_angle.data = 0;
                ROS_INFO("WP Mission Completed");

                if(target_string != destination_data){

                    wp_go_id = 0;

                    if(target_string == "init"){ init_waypoint(); } ////////////////////////////////////////////////////////////////////////////////
                    else if( target_string == "Unitophia" ){ Unitophia();  destination_data = "Unitophia"; }
                    else if( target_string == "Multi_Media_Gwan" ){ Multi_Media_Gwan();  destination_data = "Multi_Media_Gwan"; }
                    else if( target_string == "Hak_Ye_Gwan" ){ Hak_Ye_Gwan();  destination_data = "Hak_Ye_Gwan"; }
                    else if( target_string == "BRIX_Gwan" ){ BRIX_Gwan();  destination_data = "BRIX_Gwan"; }
                    else if( target_string == "San_Hak_Hyeop_Ryeok_Gwan" ){ San_Hak_Hyeop_Ryeok_Gwan();  destination_data = "San_Hak_Hyeop_Ryeok_Gwan"; }
                    else if( target_string == "Gong_Hak_Gwan" ){ Gong_Hak_Gwan();  destination_data = "Gong_Hak_Gwan"; }
                    else if( target_string == "Library" ){ Library();  destination_data = "Library"; }
                    else if( target_string == "Antire_preneur_Gwan" ){ Antire_preneur_Gwan();  destination_data = "Antire_preneur_Gwan"; }
                    else if( target_string == "Naturel_Science_Gwan" ){ Naturel_Science_Gwan();  destination_data = "Naturel_Science_Gwan"; }
                    else if( target_string == "Humanities_Social_Science_Gwan" ){ Humanities_Social_Science_Gwan();  destination_data = "Humanities_Social_Science_Gwan"; }
                    else if( target_string == "Main_University" ){ Main_University();  destination_data = "Main_University"; }
                    else if( target_string == "Global_Village" ){ Global_Village();  destination_data = "Global_Village"; }
                    else if( target_string == "Hyang_333" ){ Hyang_333();   destination_data = "Hyang_333"; }
                    else if( target_string == "test" ) { test_multi_hakyeah();  destination_data = "test_multi_hakyeah"; }
                }
            }
        }
```
    wp_go_id > wp_finish_id 이라면 최종 목적지에 도착하여 배달이 완료되었음을 판별한다. 그리고 로봇을 정지시킨다.

    이때 기존 목적지와 다른 새로운 목적지를 sub 받으면 wp_go_id = 0으로 초기화 하고, 새로운 경로를 설정한다. 이후 start_command = true를 sub 받으면 다시 주행이 시작된다.

## 회피 알고리즘
```cpp
void avoid_function_start_Callback(const std_msgs::Bool::ConstPtr& msg)
{
    if( (avoid_function_start == false) && msg->data) { avoid_function_start = msg->data; }
}

void avoid_heading_angle_Callback(const std_msgs::Float64MultiArray::ConstPtr& msg)
{
    avoid_heading_angle_right = msg->data[0];
    avoid_heading_angle_left = msg->data[1];

}

ros::Subscriber avoid_function_start_sub = n.subscribe("/avoid_function_start",10,&avoid_function_start_Callback);
ros::Subscriber avoid_heading_angle_sub  = n.subscribe("/avoid_heading_angle",10,&avoid_heading_angle_Callback);
```
```cpp
            // avoid function
            if( (wp_go_id < wp_finish_id) && (avoid_function_start == true) && (wp_go_id != set_delivery_id) && (start_command == true) ){
                c_speed.data = 60;
                s_angle.data = avoid_heading_angle_left + 30;

                if( (avoid_heading_angle_left > 5) && (avoid_heading_angle_right > 5)) {
                    s_angle.data = car_angle - 5;
                }

                avoid_function_start = false;

                printf("----------------------------\n");
                printf("Avoid function\n");
                printf("----------------------------\n");
                ROS_INFO("steering_angle : %d Speed : %d \n",s_angle.data ,c_speed.data);
            }
```
    장애물이 감지됨을 판별하는 topic을 sub받아 if문을 통해 회피 알고리즘을 수행한다.
    if문의 내용은 다음과 같다.
    * wp_go_id < wp_finish_id 주행하는 도중에 회피가 수행되야 한다.
    * avoid_function_start == true 장애물이 감지되었을때 회피해야 한다.
    * wp_go_id != set_delivery_id 손님 배송위치에 있을때 회피가 이루어지면 안된다.
    * start_command == true 주행이 시작되고 나서 회피가 수행되야 한다.

    회피 알고리즘이 수행되면 로봇 속도를 60으로 감소시킨다.
    avoid_heading_angle_left 은 장애물의 가장 왼쪽 각도(위치)를 의미한다. 로봇은 보통 우차선을 통해 주행하므로 일반적으로 장애물의 왼쪽으로 회피가 이루어저야 한다.

    if( (avoid_heading_angle_left > 5) && (avoid_heading_angle_right > 5)) 하지만 만약 장애물의 로봇의 왼쪽에 있는 경우, 로봇은 오른쪽으로 회피해야 한다.

## 전방 사람 감지 시, 우선 정지
```cpp
void detect_name_Callback(const std_msgs::String::ConstPtr& msg)
{
    detect_name = msg->data;

}

void depth_Callback(const std_msgs::Float64::ConstPtr& msg)
{
    depth = msg->data;

}

ros::Subscriber detect_name_sub  = n.subscribe("/detect_name",10,&detect_name_Callback);
ros::Subscriber depth_sub  = n.subscribe("/depth",10,&depth_Callback);
```
```cpp
if(emergency_stop == true || ((detect_name == "person") && (depth < 2.3)) ){ 
            c_speed.data = 0;
            s_angle.data = 0;
        }
```
    사물인식 프로그램 구동 중, 사람이 감지되었고 그 거리가 2.3m 이하 일 경우 로봇을 정지시킨다.
    또한 emergency_stop 이 활성화 된 경우에도 로봇을 정지시킨다.

## 조이스틱으로 조종시 속도, 조향 명령 중지
```cpp
void start_joy_Callback(const std_msgs::Bool::ConstPtr& msg)
{
    start_joy = msg->data;

    sub_joy_current_time = ros::Time::now().toSec();
}

ros::Subscriber start_joy_sub  = n.subscribe("/start_joy",10,&start_joy_Callback);
```
```cpp
if( count>=2 && (start_joy == false) ) {

            SteerAngle_pub.publish(s_angle);
            car_speed_pub.publish(c_speed);
        }

        if(ros::Time::now().toSec() - sub_joy_current_time > 1){ start_joy = false; }
```
    조이스틱으로 조종하고 있지 않다면 기본적으로 속도, 조향 명령을 pub한다.

    조이스틱으로 조종 중이라면 명령 pub를 중단한다. (주행을 위한 계산을 계속 이루어지며, 값의 pub만 되지 않는다.)

# 2. 속도 제어 및 smooth한 가,감속을 위한 PI제어 (test_pid.cpp)
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/909e55b6-bab7-4f72-8681-801a364bd6ac)

    빨간색 선은 명령 속도이며(/desired_speed) 파란색 선은 로봇 속도 측정값이다.(/measure_speed)

    명령 속도 자체를 부드러운 sgmoid 함수 형태로 주고, 실제 속도가 빠르게 따라가도록 하는 제어가 좋은 제어이다.

    P gain = 30, I gain = 12, D gain = 0 으로 하여 PI제어를 수행한다.

    위 사진은 멀티미디어관 실내에서 연구실 앞에서 계단 뒤쪽까지 주행했을때의 명령값과 실제 속도값의 그래프이다.
    차량이 명령값을 따라 천천히 속도가 증가하여, 안정한 상태에서 속도를 유지하고 정지 명령을 받은 후의 명령값을 따라 천천히 감속하는 동작을 나타낸다.
    이처럼 부드러운 sigmoid 함수 형태의 속도 명령값을 실제 차량 속도가 따라가게끔 만들어 부드러운 가속 및 감속이 이루어지고 있다. 이를 통해 로봇의 배달 상품이 엎어지거나 쏟아지는 등의 손상을 방지할 수 있다.

## smooth한 속도 명령
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/de31a2f0-38a6-455b-ab83-525e60b9848e)

```cpp
    while (ros::ok())
    {
        if(speed_230 == true){
            // 현재 ROS 시간을 사용하여 x 값을 생성 (ROS 시간을 사용하려면 roscore가 실행 중이어야 함)
            ros::Time current_time = ros::Time::now();
            delta_time =  current_time.toSec() - init_time.toSec();  // x 값을 계산
            double x = delta_time - 4;

            // 시그모이드 함수 계산
            sigmoid_value = 1.3 / (1.0 + std::exp(-x));

            desired_speed = sigmoid_value;

        }
        else if(speed_60 == true){
            // 현재 ROS 시간을 사용하여 x 값을 생성 (ROS 시간을 사용하려면 roscore가 실행 중이어야 함)
            ros::Time current_time = ros::Time::now();
            delta_time =  current_time.toSec() - init_time.toSec();  // x 값을 계산
            double x = delta_time - 4;

            // 시그모이드 함수 계산
            sigmoid_value = 0.7 / (1.0 + std::exp(-x));

            desired_speed = sigmoid_value;
        }
        else if(speed_0 == true){
            // 현재 ROS 시간을 사용하여 x 값을 생성 (ROS 시간을 사용하려면 roscore가 실행 중이어야 함)
            ros::Time current_time = ros::Time::now();
            delta_time =  current_time.toSec() - init_time.toSec();  // x 값을 계산
            double x = delta_time - 4;

            // 시그모이드 함수 계산
            sigmoid_value = 1.3 / (0.65 + 20*std::exp(x));
            desired_speed = sigmoid_value;

        }
```
    위 "1. 실외주행 알고리즘" 에서 주행 명령을 sub 받으면 if(speed_230 == true) 문을 실행한다.

    시간에 따라 천천히 가속 및 감속하므로 x 변수에는 시간이 들어가야 한다.  delta_time =  current_time.toSec() - init_time.toSec();  // x 값을 계산

    이때 차량의 속도는 0에서 1까지 천천히 증가하므로 위의 sigmoid 함수에서 x축 시간이동을 하여 사용할 필요가 있다. double x = delta_time - 4;

    sigmoid_value = 1.3 / (1.0 + std::exp(-x));   분자값 1.3은 차량의 속도가 0에서 1.3 까지 증가하도록 하는 것이다. 안정된 속도의 값을 변경하려면 이 분자의 값을 변경하면 된다.
----------------------------------------------------------------------------------------------------------------------------------------------------------
    else if(speed_60 == true) 회피 주행시 실행된다. sigmoid 함수는 0에서 0.7까지 속도가 증가하도록 한다.

    하지만 이는 1.3으로 주행하던 로봇이 회피 기동시 속도를 0까지 떨어트리고 다시 0.7까지 증가시킨다. 실제로 회피기동을 촬영한 동영상에서도 나타났다.
    이는 현재 속도에서 0.7로 감소하도록 하는 방식으로 알고리즘을 수정할 필요가 있을 것이다.
----------------------------------------------------------------------------------------------------------------------------------------------------------
    else if(speed_0 == true) 정지 명령을 받으면 실행된다.

    속도 1.3에서 0까지 감속하며, 가속할때 보다는 빠르게 감속하도록 분모에서 20이 곱해졌다.

    그리고 회피 주행중 속도 0.7에서 0으로 감속하는 것이 아니므로 알고리즘을 수정할 필요가 있다.

## 로봇 속도 측정값
```cpp
// /odom 콜백 함수
void odom_callback(const nav_msgs::Odometry::ConstPtr& msg) {
    measure_msg = msg->twist.twist.linear.x;

    measure_speed_msg.data = measure_msg;
    measure_speed_pub.publish(measure_speed_msg);

    desire_speed_msg.data = desired_speed;
    desire_speed_pub.publish(desire_speed_msg);

    // PID 제어 수행
    control_output = speed_pid_control(measure_msg);

    // 제어 출력값을 std_msgs::Int16 유형으로 변환하여 pub
    speed_msg.data = control_output;
    speed_pub.publish(speed_msg);

    pid_error_msg.data = error;
    pid_error_pub.publish(pid_error_msg);
}

ros::Subscriber odom_sub = nh.subscribe("/odom", 10, odom_callback);
```
    henes T870 바퀴 모터의 엔코더로부터 아두이노 mega 가 encoder tick을 계산한다.(/encoder_arduino) 
    이를 openCR 이 sub받아 로봇의 속도를 계산하여 /odom 을 pub한다.
    /odom을 sub받아 PI제어의 측정값으로 사용한다.

## PI 제어
```cpp
double Kp = 0; //190;  // P 게인  // pid test indoor 70
double Ki = 12; //3.5;  // I 게인
double Kd = 0; //4; // D 게인

// 속도 PID 제어 함수
double speed_pid_control(double current_speed) {

    error = desired_speed - current_speed;
    error_integral += error;
    error_integral = (error_integral >= 255) ? 255 : error_integral;
    error_integral = (error_integral <= -255) ? -255 : error_integral;
    error_derivative = error - prev_error;

    pid_output = Kp * error + Ki * error_integral + Kd * error_derivative;

    if(speed_230 == true){

        if(current_speed >= 0.1){
            if(  (current_speed > desired_speed) || (error >= -0.05) && (error <= 0.05) ){
                error_integral = error_integral - 0.1;
            }
            else if( pid_output > 200 && current_speed > 0.8 ) {
                error_integral = error_integral - 3;
            }
        }
    }
    else if(speed_0 == true){
        if(  (current_speed >= -0.05) && (current_speed <= 0.05) ){
            error_integral = 0;
        }

    }

    prev_error = error;

    // 속도 값이 특정 범위를 벗어나면 조정
    if (pid_output > 255.0) {
        pid_output = 255.0;
    } else if (pid_output < -255.0) {
        pid_output = -255.0;
    }

    return pid_output;
}
```
    * if(  (current_speed > desired_speed) || (error >= -0.05) && (error <= 0.05) )  
    현재 속도가 명령속도보다 크거나 에러가 작을 경우 적분에러를 줄인다. 안정한 상태에서 적분 에러가 너무 크게 감소할 경우 차가 울컥거리는 문제가 있어 조금만 감소하도록 하였다.

    * else if( pid_output > 200 && current_speed > 0.8 )
    평평한 도로를 주행할때는 보통 pwm=100으로 주행하며, 멀티미디어 -> 학예관 언덕길을 올라가는 경우 pwm=255이다.
    이때 언덕을 다 올라갔을때 속도가 빨라지는 문제가 발생하는데, 이때 적분에러가 크게 감소해야 해결된다.

    * else if(speed_0 == true)
    명령속도가 0이었을 경우, 현재 속도가 0에 가까울 경우 pwm=0으로 한다.

## 알고리즘 보완점
```cpp   
    error = desired_speed - current_speed;
    error_integral += error;
    error_integral = (error_integral >= 255) ? 255 : error_integral;
    error_integral = (error_integral <= -255) ? -255 : error_integral;
    error_derivative = error - prev_error;
```
    pwm 최대 값이 255인데, 적분에러도 마찬가지로 255가 최대이다.
    뭔가 PID 제어적으로 이상하고 바꿔야 할 필요가 있는건 알겠는데 잘모르겠다.

    대충 평지에서는 pwm을 100으로 1.3만큼의 속도로 주행하고, 언덕에서는 pwm=255로 하여 1.3만큼 주행하는 속도제어 자체는 대충 된다.

    또한 기본 주행 속도 1.3 -> 정지 0 으로 smooth하게 가속 및 감속은 하지만 중간에 회피 기동이 수행되었을때 매끄럽게 동작하지 않는다. 이를 보완해야할 필요가 있다.

# 3. Lidar 장애물 위치 탐지 (lidar_detect.cpp)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/4da07c4b-23f6-40c0-9848-4221c668c9c6)

    Lidar 에게 /scan을 sub받는다. 이 중에서 감지된 물체까지의 거리는 msg.ranges[] 에 담겨있다. 이 배열은 0~1946 까지 원소값을 가지며 인덱스 값을 통해 Lidar를 기준으로 특정 위치의 거리값을 알 수 있다.

    만약 좌, 우 60도 범위에서 특정 거리보다 가까운 물체가 없다면 false, 가까운 물체가 있다면 true를 pub 하도록 하여 장애물이 탐지 되었는지 아닌지 판별할 수 있다.

    Lidar의 전방(180도)을 기준으로 좌, 우 60도 범위의 /scan 데이터를 읽어서 그 중 물체의 거리가 특정거리보다 가까울경우 해당 인덱스를 저장한다. 

    이 인덱스를 360도 범위에서 매핑하면 좌, 우 몇 도 위치에 장애물이 탐지되었는지 알 수 있다. 

    또한 장애물의 가장 오른쪽과 왼쪽의 위치를 따로 알 수 있도록 하였으며, 이는 상황에 따라 장애물의 왼쪽으로 회피할지 오른쪽으로 회피할지에 대한 알고리즘에 사용된다.

## 전방 기준 좌, 우 일정 범위의 데이터 발췌
```cpp 
    for(int i = -330; i <= 330; i++){
        size = msg->ranges.size();
        INDEX = i + size/2;
```

## 장애물의 오른쪽, 왼쪽 Index 추출
```cpp
        if(msg->ranges[INDEX] <= 2.7) {
            if(detect_count == 0) { object_right_angle_index = INDEX; }
            else if (detect_count != 0) { object_left_angle_index = INDEX; }
            detect_count ++;
        }
```
    Lidar는 반시계 방향으로 회전하고 있다. 그러므로 처음으로 장애물이 감지된다면 그것은 물체의 가장 오른쪽일 것이다.

    그 다음으로 장애물이 감지된다면 object_left_angle_index 를 해당 Index로 계속해서 업데이트 한다. 만약 장애물이 더이상 탐지되지 않거나 왼쪽 60도까지 스캔이 완료되면 가장 마지막으로 저장된 Index가 장애물의 가장 왼쪽을 나타내는 것이다.

## Index를 위치(각도)로 변환
```cpp    
    object_right_angle = map(object_right_angle_index, 0, size-1, 0, 360) - 180;
    object_left_angle = map(object_left_angle_index, 0, size-1, 0, 360) - 180;
```

## 일정시간이 지나면 회피 기동 명령 pub
```cpp   
    // avoid_function
    if( detect_msg.data == true && detection_start_time.isZero() ) { detection_start_time = ros::Time::now(); }
    if( detect_msg.data == false ) { detection_start_time = ros::Time(0); }

    if(detect_msg.data && (ros::Time::now() - detection_start_time).toSec() >= 0.1 ) {

        avoid_function_msg.data = true;
        avoid_function_start_pub.publish(avoid_function_msg);
    }
    else if(!detect_msg.data || (ros::Time::now() - detection_start_time).toSec() <= 0.1 ) { 

       avoid_function_msg.data = false; 
       avoid_function_start_pub.publish(avoid_function_msg);
    }
```
    사실 원래는 장애물이 감지되었고 그 각도를 pub 하는것은 계속 반복되고 있는 상태에서, 장애물이 감지된지 일정 시간이 지나면 회피 기동 명령을 따로 pub 하도록 하려고 하였다.

    이를 통해 장애물이 감지되면 일단 정지하고, 정지한지 5초 정도가 지나면 "잠시만 비켜주세요" 안내 멘트가 출력되고 그 다음에 회피 기동이 이루어지게 만들려고 했다.

    하지만 감지된지 0.1초 만에 회피기동이 이루어지게 사용하는 중이다. 필요에 따라 코드를 수정하여 원하는 동작을 하도록 하면 될 것이다.

# 4. Depth Camera 사물인식 (detect.py)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/d7749a5d-5cb5-4a35-9b3d-947f00569b23)

    사물인식 프로그램이 실행되면, 인식된 사물 주위에 네모 박스가 둘러지고 사물의 이름이 라벨링 된다.

    라벨링된 사물의 이름이 "person"이고 그 Depth 값을 가져와 사람이 특정 거리보다 가까이 있을 경우 정지하도록 알고리즘을 구현하였다.

    마침 프로그램에서 인식된 사물의 이름과 네모칸의 좌표, 그 거리값을 변수에 저장하므로 이를 통해 구현하였다.

## 인식된 사물 이름과 거리값 pub 하도록 ros 추가
```py 
import rospy

       rospy.init_node('detect_publisher')
       depth_pub = rospy.Publisher('/depth', Float64, queue_size=10)
       detect_name_pub = rospy.Publisher('/detect_name', String, queue_size=10)
```

## 알고리즘
```py
                  if names[c] == 'person':
                       x_center = (xyxy[0] + xyxy[2]) / 2
                       y_center = (xyxy[1] + xyxy[3]) / 2
                       depth_value = depth_frame.get_distance(int(x_center), int(y_center))
                       depth_msg = depth_value
                       depth_pub.publish(depth_msg)

                       detect_name_msg = label
                       detect_name_pub.publish(detect_name_msg)
```
    Camera를 통해 사물의 이름이 라벨링되는데, 그 이름이 "person"일 경우 알고리즘이 수행된다.

    주위에 둘러진 네모칸의 꼭짓점 좌표는 xyxy[] 배열의 0~3에 저장되어있다. 이를 통해 네모 박스의 중심좌표를 알 수있다.

    그 좌표의 depth값을 가져오며, 이를 pub한다.

    이것은 실외주행 알고리즘에서 sub되어 사람이 가까이 있을경우 정지하는 동작을 수행한다.

# 5. 조향 명령 세분화를 통한 급 조향 방지 (angle_segmentation.cpp)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/54121e56-bf1e-46b9-8915-026fa379b86a)

    GPS와 다음 경유점까지의 상대좌표 계산을 통해 조향 각도가 계산되어 /Car_Control_cmd/SteerAngle_Int16 를 sub 받는다.

    기존에는 위 조향각도를 바로 아두이노가 sub받아 PID제어를 통해 모터를 제어한다. 이때 PID 게인값을 적절히 튜닝하여 1도 단위로 조향이 가능하도록 하였다.

    하지만 바퀴의 현재 각도가 0도에서 30도까지 조향하도록 명령받으면 조향이 급격하게 이루어지며 이는 주행 로봇의 안정성을 저하시키는 요인이 되었다.

    그래서 명령 조향각도를 smooth하게 제공하는 알고리즘을 구현하였다.

## 실외주행 노드로 부터 조향각 sub
```cpp
void SteerAngle_Callback(const std_msgs::Int16::ConstPtr& msg)
{
    steer_angle_current = msg->data;
}

    ros::Subscriber SteerAngle_sub = nh.subscribe("/Car_Control_cmd/SteerAngle_Int16", 10, SteerAngle_Callback);
```

## 각도 증가율 결정
```cpp
        if( steer_angle_current == (sum + 1) || steer_angle_current == (sum - 1) ) { alpha = 1; } 
        else if(steer_angle_current == (sum + 2) || steer_angle_current == (sum - 2) ) { alpha = 2; } 
        else if(steer_angle_current > (sum + 3) || steer_angle_current < (sum - 3) ) { alpha = 3; } 
```
    * steer_angle_current 는 실외주행 노드로부터 최종적으로 조향해야 하는 명령 조향각도이다.
    * sum 은 steer_angle_current까지 점차 증가하는 세분화된 각도이다. 이 값을 실제 바퀴를 제어할 아두이노로 pub 한다.
    * alpha 는 sum을 얼마나 증가시킬지에 대한 변수이다. 조건에 따라 alpha값이 결정된다.

    만약 현재 조향각이 0도이고, 명령 조향각도가 8이라면 세분화 각도는 0, 3, 6, 8 도 이런식으로 증가한다.
    또는 현재 조향각=2, 명령 조향각=12 일 경우 세분화 각도는 2, 5, 8, 11, 12 이렇게 증가하여 아두이노로 pub 한다.

## 조향 방향 판단
```cpp
        if(steer_angle_current - sum > 0) { msg_positive = true; }
        if(steer_angle_current - sum < 0) { msg_positive = false; }
```
    만약 현재 조향각=0, 명령 조향각=10 일 경우 알고리즘에 의해 세분화 조향각은 10이 되었을 것이다.

    이때 명령 조향각이 -10 이 된다면 세분화 조향각은 10에서 -10으로 줄어들어야 할 것이다. 이를 판단하는 코드이다.

## 조향각 세분화 알고리즘 수행
```cpp
        if(msg_positive == true){
            if(steer_angle_current != sum){
                sum += alpha;
                steer_angle_msg.data = sum;
                SteerAngle_pub.publish(steer_angle_msg);
            }
            else if(steer_angle_current == sum){

                steer_angle_msg.data = sum;
                SteerAngle_pub.publish(steer_angle_msg);
            }
        }
        else if(msg_positive == false){
            if(steer_angle_current != sum){
                sum -= alpha;
                steer_angle_msg.data = sum;
                SteerAngle_pub.publish(steer_angle_msg);
            }
            else if(steer_angle_current == sum){

                steer_angle_msg.data = sum;
                SteerAngle_pub.publish(steer_angle_msg);
            }
        }
```

## 보완점
```cpp
ros::Rate loop_rate(5);
```
    연산 속도가 5Hz이다. 위에서 설명한 각도 세분화 알고리즘을 수행함에 있어서 딜레이를 주지 않았기 때문에 알고리즘이 너무 빠르게 수행되어 아무런 효과가 없을것이다. 이를 보완할 필요가 있다.


# 6. GPS 칼만필터 적용 (gps_KalmanFilter.cpp)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/907e964d-9f36-4f1e-b28d-7d383ec411d0)

    위 사진에서 사용된 코드는 실제 사용한 코드가 아니다.

    칼만필터와 class 에 대해 공부하면 이해하는데 도움이 될 것이다.

## 경도, 위도 칼만필터 객체를 선언하고 초기화 한다.
```cpp
class KalmanFilter {
public:
    KalmanFilter(double initial_state, double measurement_noise, double process_noise, double initial_covariance) :
        state_(initial_state),
        covariance_(initial_covariance),
        process_noise_(process_noise),
        measurement_noise_(measurement_noise) {}

    GPSToKalmanNode() : nh_("~"), kalman_filter_lat_(0.0, 0.01, 0.1, 1.0), kalman_filter_lon_(0.0, 0.01, 0.1, 1.0) {
```

## GPS 값 sub
```cpp
        sub_ = nh_.subscribe("/ublox_gps/fix", 1, &GPSToKalmanNode::gpsCallback, this);

    void gpsCallback(const sensor_msgs::NavSatFix::ConstPtr& msg) {
        // Apply Kalman filter to GPS latitude and longitude, considering position_covariance
        double filtered_latitude = kalman_filter_lat_.filter(msg->latitude, msg->position_covariance[0]);
        double filtered_longitude = kalman_filter_lon_.filter(msg->longitude, msg->position_covariance[4]);
```
    GPS 데이터인 /ublox_gps/fix 을 sub하여 경도와 경도 공분산, 그리고 위도와 위도 공분산 값을 각각 칼만필터를 수행한다.

## 칼만필터 수행
```cpp
    double filter(double measurement, double position_covariance) {

        // Prediction
        double predicted_state = state_;
        double predicted_covariance = covariance_ + process_noise_;

        // Calculate Kalman Gain with weighted position_covariance
        double kalman_gain = predicted_covariance / (predicted_covariance + measurement_noise_);

        // Update
        state_ = predicted_state + kalman_gain * (measurement - predicted_state);
        covariance_ = (1 - kalman_gain) * predicted_covariance;

        return state_;
    }
```
    * double predicted_state = state_; 처음 KF가 수행될때는 초기값으로 설정한 0이 사용된다.(0이 아닌 멀티미디어 경도, 좌표로 설정해야 한다.) 그 이후로는 이전에 계산한 예측값으로 사용한다.

    * double predicted_covariance = covariance_ + process_noise_; 처음은 초기값, 이후로는 이전에 계산한 공분산값을 사용한다. 이때 process_noise_값은 시스템 노이즈값을 의미하는데 알 수 없으므로 상수값 0.1을 사용한다.

    * double kalman_gain = predicted_covariance / (predicted_covariance + measurement_noise_); 칼만이득을 계산한다. 이때 measurement_noise_는 센서 노이즈를 의미하며 실제 GPS를 통해 sub받은 경도 또는 위도의 공분산값을 사용한다.
  GPS 공분산이 크다면 칼만이득은 감소하고, 공분산이 작다면 칼만이득은 증가한다.

    * state_ = predicted_state + kalman_gain * (measurement - predicted_state); 칼만필터를 수행하여 필터된 경도, 위도값을 계산한다. 여기서 measurement는 GPS를 통해 sub받은 경,위도 값이다.
  예측값, 실제 GPS값, 칼만이득을 통해 칼만필터가 적용된 상태값을 계산한다.

    * covariance_ = (1 - kalman_gain) * predicted_covariance;  칼만이득과 예측 공분산값으로 공분산을 업데이트 한다. 이는 다음 칼만필터 과정에서 사용된다.

## KF된 값 pub
```cpp
        double filtered_latitude = kalman_filter_lat_.filter(msg->latitude, msg->position_covariance[0]);
        double filtered_longitude = kalman_filter_lon_.filter(msg->longitude, msg->position_covariance[4]);

        // Update previous values
        //prev_latitude_ = filtered_latitude;
        //prev_longitude_ = filtered_longitude;

        // Create filtered GPS message
        sensor_msgs::NavSatFix filtered_gps;
        filtered_gps.header = msg->header;
        filtered_gps.latitude = filtered_latitude;
        filtered_gps.longitude = filtered_longitude;
        filtered_gps.position_covariance = msg->position_covariance;
        filtered_gps.position_covariance_type = msg->position_covariance_type;

        // Publish the filtered GPS message
        pub_.publish(filtered_gps);
```

## 보완점

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/3037ff3e-91cd-48f4-adf6-50db8fb2e3c6)

```cpp
    GPSToKalmanNode() : nh_("~"), kalman_filter_lat_(0.0, 0.01, 0.1, 1.0), kalman_filter_lon_(0.0, 0.01, 0.1, 1.0) {
```
    여기서 경도와 위도의 초기값을 0으로 주었는데 이것을 멀티미디어관의 경, 위도로 대입하는것이 좋다.

    만약 0이라면 필터링이 수행되면서 점차 실제 위치로 계산되는것인데

    현재 모든 알고리즘을 하나의 명령어로 수행시키게끔 단일 패키지화 시켰는데, 처음 실외주행이 시작될때 동시에 칼만필터 알고리즘이 실행된다.
    이때 너무 빠르게 로봇을 출발시키면 KF 필터링이 제대로 되지 않은 단계에서 현재위치를 제대로 추정하지 못하는 문제가 발생할 것으로 예상된다.

    이것을 수정할 필요가 있다.

# 7. 모터 제어 아두이노 mega (sandle_arduino_3.ino)

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/d6fb0491-f52b-4b2a-a392-ea06f396254e)

    아두이노는 전륜, 후륜, 조향 모터를 제어하기 위해 3개의 모터드라이버(DCMD-200-P)에 연결되어 있다.

    또한 바퀴 엔코더를 통해 tick을 받아오며, 조향모터의 각도를 받아온다.

    실외주행 코드로부터 전진, 정지, 조향 등의 명령을 sub받으면 그 조건에 따라 모터드라이버에 연결된 pwm, dir 핀에 다른 값을 출력한다.

    그럼 모터드라이버가 실제 모터에게 인가하는 전압을 달리하여 모터를 제어한다.

## 연결도

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/84d2f097-98e7-4cbd-8686-025845a072ce)
![image](https://github.com/kimjunho654/henes_develop/assets/105560901/4bb4f1ca-81c8-4948-9c1c-1505d573d7bf)

    전륜, 후륜, 조향모터와 아두이노, 모터드라이버, 배터리는 위의 사진과 같이 연결되어 있다.

    위 사진은 후륜모터의 경우이며, 전륜과 조향은 다른 핀에 연결되어 있다.

## 엔코더 선

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/bd26a7f0-8abf-4bda-97c7-5a3aca4bb583)

    #define encoder0PinA 2  // interrupt 4 use
    #define encoder0PinB 3  // interrupt 5 use

    파란색, 초록색 선이 엔코더 A상, B상이다. 아두이노의 2, 3번 핀에 연결한다.

    로봇을 앞으로 전진 시켰는데 /odom의 속도가 - 음수로 증가한다면 핀을 바꿔서 연결하면 된다.

## 조향센서 선

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/c4648d79-ef6b-4ec9-ade1-921e77f5f59f)

    #define Steering_Sensor A8  // Analog input pin that the potentiometer is attached to

    주황색 선을 아두이노 A8에 연결한다.

## 속도 명령 sub

```ino
void speed_callback(const std_msgs::Int16& msg) {

    velocity = msg.data;
    sub_speed_prev_time = millis();

}

ros::Subscriber<std_msgs::Int16> speed_sub("PID_car_speed", speed_callback); //Car_Control_cmd/Speed_Int16
```
    실외주행 노드 --> 속도 PID 노드 --> 아두이노  이런 흐름으로 아두이노가 속도 명령을 sub 받는다.

## 전륜, 후륜 모터 제어 함수 실행
```ino
  f_speed = r_speed = velocity;
  motor_control(r_speed, r_speed);
```
    sub받은 pwm값(=velocity)을 motor_control( , ) 함수에 전달한다.
----------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
void motor_control(int front_speed, int rear_speed)
{
  front_motor_control(front_speed);
  rear_motor_control(rear_speed);  
}
```
    pwm값을 전륜, 후륜 각각의 제어 함수로 전달한다.
----------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
// Front Motor Drive
#define MOTOR1_PWM 4
#define MOTOR1_DIR 5
#define MOTOR1_BRK 6

// Rear Motor Drive
#define MOTOR3_PWM 7
#define MOTOR3_DIR 8
#define MOTOR3_BRK 9
```
    전륜모터를 담당하는 모터드라이버의 pwm, dir, brk 핀은 아두이노의 4, 5, 6 핀에 연결

    후륜모터를 담당하는 모터드라이버의 pwm, dir, brk 핀은 아두이노의 7, 8, 9 핀에 연결
----------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
void front_motor_control(int motor1_pwm)
{
   if (motor1_pwm > 0) // forward
  {
    digitalWrite(MOTOR1_DIR, HIGH);
    digitalWrite(MOTOR1_BRK, LOW);
    analogWrite(MOTOR1_PWM, motor1_pwm);
  }
  else if (motor1_pwm < 0) // backward
  {
    digitalWrite(MOTOR1_DIR, LOW);
    digitalWrite(MOTOR1_BRK, LOW);
    analogWrite(MOTOR1_PWM, -motor1_pwm);
  }
  else
  {
    digitalWrite(MOTOR1_DIR, LOW);
    digitalWrite(MOTOR1_BRK, LOW);
    digitalWrite(MOTOR1_PWM, 0);
  }
}
```
    * 전륜모터 기준 

    analogWrite(MOTOR1_PWM, motor1_pwm); sub 받은 pwm 값을 모터드라이버의 pwm 핀으로 전달한다. 이때 후진의 경우 명령값이 음수인데, 속도는 같으므로 sub받은 pwm값에 - 마이너스 부호를 붙인다. 정지의 경우 pwm 값은 0으로 한다.

    digitalWrite(MOTOR1_DIR, HIGH); sub 받은 pwm 값이 양수이면 전진, 음수이면 후진을 해야 하므로 실제 동작과 명령이 일치하도록 HIGH 또는 LOW 로 설정하면 된다.

    digitalWrite(MOTOR1_BRK, LOW); 모터드라이버에서 브레이크 기능을 제공하는 것 같다. 일반적으로 사용하지 않으므로 LOW 값으로 설정한다. 그렇다고 이 함수를 사용하지 않으면 모터가 움직이지 않으므로 LOW로 함수는 사용해야 한다.

## 조향 명령 sub
```ino
void heading_angle_callback(const std_msgs::Int16& msg) {

   steer_angle = msg.data;

}

ros::Subscriber<std_msgs::Int16> heading_angle_sub("heading_angle", heading_angle_callback);
```
    실외주행 노드 --> 조향각 세분화 노드 --> 아두이노  이런 흐름으로 아두이노가 조향 명령을 sub 받는다.

## 조향 제어 함수 실행
    void setup() 문에서 MsTimer2::set(200, control_callback);  0.2초 마다 자동으로 조향 제어 함수가 실행된다.

    void loop() 문에서 control_callback(); 조향 제어 함수가 실행된다.
---------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
#define Steering_Sensor A8  // Analog input pin that the potentiometer is attached to

void control_callback()
{

  motor_control(f_speed, r_speed);

  // read the analog in value:
  sensorValue = analogRead(Steering_Sensor);
  // map it to the range of the analog out:
  Steer_Angle_Measure = map(sensorValue, 72.0, 1023.0, LEFT_STEER_ANGLE, RIGHT_STEER_ANGLE);
  Steering_Angle = NEURAL_ANGLE + steer_angle;
  steering_control();

}
```
    Steering_Sensor(아두이노 A8 핀)으로 부터 조향 센서 값을 읽어온다. 그 값이 sensorValue 이다.

    센서 값은 0~1023의 값을 가진다. 이것을 좌, 우 20도. -20 ~ 20 의 범위로 매핑한다. 그 값이 Steer_Angle_Measure 이다.

    Steering_Angle 은 조향 명령값이다. (NEURAL_ANGLE = 0)

    steering_control() 조향 제어 함수를 실행시킨다.
---------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
void steering_control()
{
  if (Steering_Angle <= LEFT_STEER_ANGLE + NEURAL_ANGLE)  Steering_Angle  = LEFT_STEER_ANGLE + NEURAL_ANGLE;
  if (Steering_Angle >= RIGHT_STEER_ANGLE + NEURAL_ANGLE)  Steering_Angle = RIGHT_STEER_ANGLE + NEURAL_ANGLE;
  PID_Control(); 
}
```
    명령 조향 각도가 너무 크거나 작다면 제한한다.

    PID_Control();  현재 조향 각도를 명령 조향 각도까지 위치제어하는 함수를 실행시킨다. 

## 조향 위치 제어

```ino
float Kp = 3.0;
float Ki = 2.0;
float Kd = 10.0; //PID 

void PID_Control()
{
  error = Steering_Angle - Steer_Angle_Measure ;
  error_s += error;
  error_d = error - error_old;
  error_s = (error_s >=  80) ?  80 : error_s;
  error_s = (error_s <= -80) ? -80 : error_s;

  pwm_output = Kp * error + Kd * error_d + Ki * error_s;
  pwm_output = (pwm_output >=  255) ?  255 : pwm_output;
  pwm_output = (pwm_output <= -255) ? -255 : pwm_output;

  if (error == 0)
  {
    steer_motor_control(0);
    error_s = 0;
  }
  else          steer_motor_control(pwm_output);
  error_old = error;  
}
```
    현재 조향 각도와 명령 조향 각도가 차이가 있는 경우, PID에 의해 pwm값을 조향 모터 제어 함수에 전달한다. steer_motor_control(pwm_output);

    만약 현재와 명령 조향 각도가 일치한다면 pwm을 0으로 하여 조향 모터 제어 함수에 전달한다. steer_motor_control(0);
---------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
void steer_motor_control(int motor_pwm)
{
  if (motor_pwm > 0) // forward
  {
    digitalWrite(MOTOR2_DIR, LOW);
    digitalWrite(MOTOR2_BRK, LOW);
    analogWrite(MOTOR2_PWM, motor_pwm);
  }
  else if (motor_pwm < 0) // backward
  {
    digitalWrite(MOTOR2_DIR, HIGH);
    digitalWrite(MOTOR2_BRK, LOW);
    analogWrite(MOTOR2_PWM, -motor_pwm);
  }
  else // stop
  {
    digitalWrite(MOTOR2_DIR, LOW);
    digitalWrite(MOTOR2_BRK, LOW);
    analogWrite(MOTOR2_PWM, 0);
  }
}
```
    조향 모터 제어 함수는 앞서 설명한 전륜, 후륜 모터 제어 함수와 동작이 동일하다.

## 엔코더 값 읽어오기
```ino
#define encoder0PinA 2  // interrupt 4 use
#define encoder0PinB 3  // interrupt 5 use

  pinMode(encoder0PinA, INPUT_PULLUP); 
  pinMode(encoder0PinB, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(encoder0PinA), doEncoderA, CHANGE); // encoder pin on interrupt 4 (pin 2)
  attachInterrupt(digitalPinToInterrupt(encoder0PinB), doEncoderB, CHANGE); // encoder pin on interrupt 5 (pin 3)
```
    엔코더 선을 아두이노 2, 3번 핀에 연결한다. 아두이노의 2, 3번 핀은 각각 인터럽트 4번, 5번을 담당한다.

    2, 3번 핀의 논리값이 HIGH -> LOW 그리고 LOW -> HIGH 가 될때 인터럽트를 실행한다. CHANGE

    각각 실행될 인터럽트 함수는 doEncoderA, doEncoderB 이다.

## 인터럽트 함수 실행

![image](https://github.com/kimjunho654/henes_develop/assets/105560901/179106be-0855-45e9-9fc0-fb2245031d7f)

```ino
volatile int encoder0Pos = 0;

void doEncoderB(){
  if (digitalRead(encoder0PinB) == HIGH) {   
    if (digitalRead(encoder0PinA) == HIGH) {  
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
  else { 
    if (digitalRead(encoder0PinA) == LOW) {   
      encoder0Pos = encoder0Pos + 1;
    } 
    else {
      encoder0Pos = encoder0Pos - 1;
    }
  }
}
```
    위 그림을 기준으로 B상이 LOW -> HIGH 되어 인터럽트가 발생했고, B상이 HIGH 일때 A상이 HIGH 이면 모터가 시계방향으로 돌아갔음을 알 수 있다. 그의 반대는 모터가 반시계 방향으로 돌아갔다는 의미이다.

    마찬가지로 A상이 LOW -> HIGH 되어 인터럽트 발생, A상이 HIGH 일때 B상이 HIGH 이면 모터가 반시계 방향으로 돌아갔음을 알 수 있고, 그의 반대는 모터가 시계 방향으로 돌아갔음을 알 수 있다.
---------------------------------------------------------------------------------------------------------------------------------------------------------
```ino
ros::Publisher encoder_pub("encoder_arduino", &encoder_data);

  encoder_data.data = encoder0Pos;
  encoder_pub.publish(&encoder_data);
```
    이 엔코더 TICK 을 pub 하여 openCR 보드가 sub 한다.

    openCR은 이를 통해 odometry를 계산하여 /odom 을 pub한다.

    이를 속도 PID 제어 노드가 sub 하여, 로봇 속도 측정값으로 사용하고 이를 통해 PID 제어를 통한 smooth한 가속 및 감속과 속도 제어를 수행한다.



















    
