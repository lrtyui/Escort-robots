# Escort-robots

Intelligent companion robot

abstract

At present, China's population aging is accelerating, the number of empty nest elderly is increasing, and the growth rate is fast, and the proportion of the elderly population is rapidly expanding. The China Aging Research Center revised the prediction that the empty nest rate will reach 89.6% in 2030, forming about 254 million empty nest elderly groups, which is equivalent to one empty nest elderly in every 3.5 Chinese people at that time. They face many problems such as life care, emotional companionship, and health management. If there is a risk of cognitive impairment, the incidence of mild cognitive impairment in the population aged 65 and above is 20.8%; Psychological health problems occur frequently, with a depression symptom detection rate of 39.2% and a suicide rate of 82.5/100000 among elderly people living alone; There is a gap in security measures, with an average of 523000 elderly people missing each year and 72% not wearing positioning devices.

In order to meet the multi-dimensional challenges of the aging society, this work is based on the RDK X5 edge computing platform to create an intelligent service robot that integrates health monitoring, safety protection and emotional companionship. This robot is supported by humanized design and advanced technology, integrating multiple functions such as voice interaction, target following, posture recognition, health monitoring, and home safety. It aims to provide comprehensive intelligent companionship and services for the elderly and children, alleviate social elderly pressure, and enhance educational experience. The product adopts a tracked chassis design, integrates DeepSeek big language model and multimodal decision closed-loop system, and has four core advantages:

Intelligent interaction system: integrating voice emotion analysis and visual recognition technology, supporting natural dialogue and multimodal interaction, equipped with emotion perception and active care functions.

Global mobility: Innovative tracked chassis design (obstacle crossing height ≥ 6cm) combined with a 3D environment perception system to achieve autonomous navigation and target tracking in complex terrains.

Health and Safety Center: Integrated multi parameter physiological monitoring (heart rate/blood oxygen/fall detection) and environmental sensors (smoke/gas), supporting real-time warning and remote monitoring.

Modular service platform: Through ROS architecture to achieve functional expansion, it can adapt to the needs of medical, educational and other scenarios, and support multi-user collaborative management.

The application scenarios cover home-based elderly care (empty nest elderly care), community services (health data management), medical rehabilitation (remote monitoring), and children's education. After testing, the target tracking accuracy reaches ± 5cm, the health monitoring response time is less than 3s, and it has the ability to replace 15% of the traditional elderly care equipment market, providing an intelligent solution for cracking the "9073" elderly care pattern.

Part One Overview of the Work

- 1. Function and Features

Functional flowchart

Multimodal interaction: Voice interaction supports natural language processing and sentiment analysis, understanding and responding to user commands; Visual interaction supports target recognition and environmental perception; Action interaction combines robotic arms and sensors, supporting simple physical interactions.

Multi modal interactive data flow chart

Crawler chassis design: capable of autonomous navigation and target following, ensuring stable tracking of elderly people in different environments, while providing a smoother operating experience, with high reliability and adaptability.

Health and safety module: capable of real-time collection of physiological data such as heart rate and blood oxygen of the elderly, monitoring indoor environmental parameters, and triggering warning mechanisms when abnormalities are detected.

Remote control: Using PICO to remotely control robots for simple action demonstrations, providing an immersive interactive experience that can be used for virtual companionship and entertainment, safety monitoring, psychological counseling, and more.

- 1. Application Fields

Elderly people living alone and their children: Robots have functions such as voice interaction, health monitoring, and target following to improve the quality of life of the elderly, meet their needs for taking care of their lives, ensuring safety, and providing emotional care.

Elderly care institutions: Intelligent companion robots can assist in completing care work and reduce the workload of nursing staff. Real time monitoring of the physical indicators of the elderly, providing entertainment, education, and social interaction functions, enriching the lives of the elderly, improving service quality, and creating a diversified elderly care environment.

Hospital Rehabilitation Department: Intelligent companion robots can assist in providing personalized rehabilitation training plans for patients, monitor training data in real-time, and adjust intensity. In daily nursing, assist in completing simple nursing tasks, provide emotional support, promote the rehabilitation process, and meet the needs of hospitals to improve rehabilitation treatment effectiveness and patient nursing experience.

Children's education market: Intelligent companion robots, as educational partners, can provide knowledge in multiple fields and meet children's thirst for knowledge. Through interactive learning, enhance the fun of learning and help children better understand and master knowledge.

Smart home market users: Robots integrate smart home device control functions, which can control household appliances, lighting and other devices, monitor real-time indicators such as temperature, humidity, and air quality in the home environment, and ensure the safety and comfort of the home environment.

- 1. Main technical features

Within 400 words;

Human recognition tracking: By capturing images through a camera, YOLOv5 is used for object detection, combined with 3D environmental perception systems (such as LiDAR and depth cameras) to obtain environmental information. SLAM (Simultaneous Localization and Mapping) algorithm is used to construct an environmental map, calculate the target position and distance, and send control instructions to make the robot move, thereby achieving the function of following specific targets. The tracked chassis adopts STM32 microcontroller for motion control. By optimizing the motion control algorithm, the robot's obstacle crossing ability and stability in complex terrain are improved.

Attitude detection: Through human keypoint detection and pedestrian detection and tracking, PPHUBAN adopts the cloud edge integrated high-precision real-time detection algorithm PP-YOLOE, which is jointly trained with multi scene open source datasets to recognize the behavior and human attributes of the elderly, and monitor whether the elderly have abnormal falling postures.

Emotion computing enhancement: Combining speech emotion analysis (intonation recognition) with facial expression recognition (CNN+attention mechanism), the model is based on convolutional neural networks (CNN) and uses Mel frequency cepstral coefficients (MFCCs) as audio features. The accuracy of emotion label classification has been improved to over 90%, enabling robots to better perceive user emotions and provide more thoughtful services.



-Main innovation points s

1. Integrate AI voice interaction, connect with DeepSeek big model, provide intelligent dialogue and professional answers, solve communication barriers for the elderly, and improve the convenience of life.
2. The tracked chassis achieves autonomous movement and target following, adapts to complex terrain, and runs stably and reliably.
3. Integrate Baidu PP Human posture detection to identify abnormal falls.
4. Equipped with DHTE temperature and humidity sensors, and later added MQ2 smoke sensors to monitor environmental safety.
5. Support multimodal interaction, combining voice, visual, and robotic arm motion control to improve interaction flexibility; Enhance service intimacy through emotional computing.
6. Develop remote control, which can be embodied and remotely operated through VR, making it convenient for family members to monitor the robot.
7. Design process

As shown in the figure, in terms of hardware, STM32 is selected for chassis motion control to meet high-precision driving requirements; Using Jetson Nano as the upper computer, deploy lightweight visual model (MobileNet) and speech sentiment analysis model (Wav2Vec). At the software level, integrate DeepSeek big language model to achieve intelligent dialogue and knowledge answering; Introduce Baidu PP Human posture detection technology (PP-YOLOE algorithm) to detect abnormal falling posture; Monitor environmental data using DHTE temperature and humidity sensors and MQ2 smoke and gas sensors added later. By using multimodal interaction technology, integrating multiple interaction methods such as voice, vision, and action, PICO remote control is added to enhance user experience.

Part 2 System Composition and Function Description

1. 1. Overall Introduction

This work adopts the technical roadmap framework of "perception decision execution". The perception layer collects environmental, user behavior, and physiological data through various sensors such as cameras, microphones, physiological monitoring sensors, and environmental sensors; The decision-making layer utilizes the DeepSeek big language model and multimodal decision-making algorithm to analyze and process the collected data, generating corresponding instructions; The execution layer is composed of hardware devices such as tracked chassis and robotic arms that execute instructions, achieving functions such as robot movement and interaction.

RDK Function Introduction

- 1. Software System Introduction

2.3.1 Overall software introduction (including PC or cloud, combined with key images);

2.3.2 Introduction to each module of the software (provide specific design instructions for each module based on the overall diagram. Provide flowcharts of each function and their key input and output variables from top to bottom);

Scalable areas

Further deepen the application scenarios of remote control robots, by deeply integrating virtual reality (VR) technology with more precise and complex motion control systems, achieving a dual breakthrough in robot operation accuracy and interactive immersion, injecting new dynamic energy into the expansion of robot functional boundaries.

Support multiple family members to log in to the system simultaneously through a mobile app, achieving remote multi terminal collaborative monitoring. Family members can view the real-time operation status of the robot and indoor real-time images, and have high-definition conversations with the elderly through voice channels to build a distributed care network.

Based on the deep linkage of IoT interfaces with smart home systems, combined with environmental perception data to dynamically adapt to scene requirements, remote APP synchronization supports home appliance status monitoring and pre setting, allowing users to control their home environment at any time and build a full scene intelligent living loop. Empowering life convenience with technology and expanding the practical value of robots in home scenarios.

