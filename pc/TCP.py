import socket
import cv2
import numpy as np
import threading
import tkinter as tk
from PIL import Image, ImageTk
import struct
import dlib
import torch
import torch.nn.functional as F
import torchvision
from torchvision import transforms
from torch.autograd import Variable
import time

import hopenet
import utils

original_width, original_height = 1920, 1080
new_width, new_height = 640, 360

MIN_BOX_SIZE = 69  # 最小框尺寸阈值

class ImageServer:
    def __init__(self, host='0.0.0.0', port=8888):
        self.host = host
        self.port = port
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        self.running = True
        self.clients = []
        self.paused = False  # 用于标记暂停状态
        self.face_detected = False  # 标记是否检测到人脸
        self.face_disappear_timer = None  # 定时器

        self.root = tk.Tk()
        self.root.title("Image Server")
        self.root.geometry("800x600")
        self.root.resizable(True, True)

        self.panel = tk.Label(self.root)
        self.panel.pack(fill=tk.BOTH, expand=True)

        self.text = tk.StringVar()
        self.label = tk.Label(self.root, textvariable=self.text)
        self.label.pack()

        self.status_text = tk.StringVar(value="Initial")
        self.status_label = tk.Label(self.root, textvariable=self.status_text)
        self.status_label.pack()

        threading.Thread(target=self.accept_connections).start()
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)

        # Hopenet and dlib initialization
        self.gpu = 0  # GPU ID
        self.snapshot_path = 'model/hopenet_robust_alpha1.pkl'
        self.face_model_path = 'model/mmod_human_face_detector.dat'
        self.model = hopenet.Hopenet(torchvision.models.resnet.Bottleneck, [3, 4, 6, 3], 66)
        saved_state_dict = torch.load(self.snapshot_path)
        self.model.load_state_dict(saved_state_dict)
        self.model.cuda(self.gpu)
        self.model.eval()
        self.transformations = transforms.Compose([transforms.Resize(224),
                                                   transforms.CenterCrop(224),
                                                   transforms.ToTensor(),
                                                   transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                                                        std=[0.229, 0.224, 0.225])])
        self.idx_tensor = torch.FloatTensor([idx for idx in range(66)]).cuda(self.gpu)
        self.detector = dlib.get_frontal_face_detector()

        self.yaw_buffer = []
        self.pitch_buffer = []
        self.roll_buffer = []
        self.start_time = None
        self.current_state = "Initial"
        self.confirm_start_time = None
        self.nod_stage = 0
        self.shake_stage = 0

        self.observation_sent = False  # 标志位，记录是否已经发送观察状态的指令

    def accept_connections(self):
        while self.running:
            client_socket, addr = self.server_socket.accept()
            self.clients.append(client_socket)
            threading.Thread(target=self.handle_client, args=(client_socket,)).start()

    def handle_client(self, client_socket):
        try:
            data = b""
            while True:
                packet = client_socket.recv(4096)
                if not packet:
                    break
                data += packet

            if not data:
                print("No data received")
                return

            object_box_size = struct.calcsize('iiii')
            object_box_data = data[:object_box_size]
            objBox = struct.unpack('iiii', object_box_data)

            image_data = data[object_box_size:]
            np_arr = np.frombuffer(image_data, np.uint8)
            image = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

            if image is None:
                print("Failed to decode image")
                return

            xmin, xmax, ymin, ymax = objBox

            # Adjust the coordinates based on the scaling factors
            xmin = int(xmin * (new_width / original_width))
            xmax = int(xmax * (new_width / original_width))
            ymin = int(ymin * (new_height / original_height))
            ymax = int(ymax * (new_height / original_height))

            # Update objBox with new values
            objBox = (xmin, xmax, ymin, ymax)

            if (xmin == 0 and xmax == 0 and ymin == 0 and ymax == 0) or ((xmax - xmin) < MIN_BOX_SIZE) or ((ymax - ymin) < MIN_BOX_SIZE):
                self.display_image(image)
                if self.face_detected:
                    if self.face_disappear_timer is None:
                        self.face_disappear_timer = threading.Timer(1.0, self.reset_state)
                        self.face_disappear_timer.start()
                return

            if self.face_disappear_timer is not None:
                self.face_disappear_timer.cancel()
                self.face_disappear_timer = None

            if not self.face_detected:
                self.send_hex_data_to_clients('aa5500')
                self.current_state = "选择中"
                self.face_detected = True

            cropped_image = image[ymin:ymax, xmin:xmax]
            face = Image.fromarray(cv2.cvtColor(cropped_image, cv2.COLOR_BGR2RGB))
            face = self.transformations(face)
            face = face.view(1, face.size(0), face.size(1), face.size(2))
            face = Variable(face).cuda(self.gpu)

            yaw, pitch, roll = self.model(face)
            yaw_predicted = F.softmax(yaw, dim=1)
            pitch_predicted = F.softmax(pitch, dim=1)
            roll_predicted = F.softmax(roll, dim=1)

            yaw_predicted = torch.sum(yaw_predicted.data[0] * self.idx_tensor) * 3 - 99
            pitch_predicted = torch.sum(pitch_predicted.data[0] * self.idx_tensor) * 3 - 99
            roll_predicted = torch.sum(roll_predicted.data[0] * self.idx_tensor) * 3 - 99

            angles = f"Yaw: {yaw_predicted.item():.2f}, Pitch: {pitch_predicted.item():.2f}, Roll: {roll_predicted.item():.2f}"

            if not self.paused:
                self.update_buffers(yaw_predicted.item(), pitch_predicted.item(), roll_predicted.item())

            # Draw face detection box and axis
            cv2.rectangle(image, (xmin, ymin), (xmax, ymax), (0, 255, 0), 2)
            utils.draw_axis(image, yaw_predicted, pitch_predicted, roll_predicted, tdx=(xmin + xmax) // 2, tdy=(ymin + ymax) // 2, size=(ymax - ymin) // 2)
            self.display_info(angles, image)
        except Exception as e:
            print(f"Error handling client: {e}")
        finally:
            client_socket.close()
            self.clients.remove(client_socket)

    def display_info(self, angles, image):
        self.text.set(angles)
        self.display_image(image)

    def update_buffers(self, yaw, pitch, roll):
        current_time = time.time()
        if self.start_time is None:
            self.start_time = current_time

        self.yaw_buffer.append(yaw)
        self.pitch_buffer.append(pitch)
        self.roll_buffer.append(roll)

        if len(self.yaw_buffer) > 0 and len(self.pitch_buffer) > 0:
            avg_yaw = sum(self.yaw_buffer) / len(self.yaw_buffer)
            avg_pitch = sum(self.pitch_buffer) / len(self.pitch_buffer)

            if self.current_state == "选择中":
                duration = current_time - self.start_time
                if avg_yaw < -5 and avg_pitch < 4:
                    if duration >= 1 and duration < 5:
                        self.status_text.set("观察左下")
                        if not self.observation_sent:
                            self.send_hex_data_to_clients('aa550100')
                            print("0100")
                            self.observation_sent = True
                    elif duration >= 5:
                        self.status_text.set("注视左下")
                        self.send_hex_data_to_clients('aa551000')
                        print("注视左下")
                        self.enter_confirmation_state()
                elif avg_yaw < -5 and avg_pitch > 8:
                    if duration >= 1 and duration < 5:
                        self.status_text.set("观察左上")
                        if not self.observation_sent:
                            print("0101")
                            self.send_hex_data_to_clients('aa550101')
                            self.observation_sent = True
                    elif duration >= 5:
                        self.status_text.set("注视左上")
                        self.send_hex_data_to_clients('aa551001')
                        print("注视左上")
                        self.enter_confirmation_state()
                elif avg_yaw > 5 and avg_pitch > 8:
                    if duration >= 1 and duration < 5:
                        self.status_text.set("观察右上")
                        if not self.observation_sent:
                            self.send_hex_data_to_clients('aa550110')
                            self.observation_sent = True
                    elif duration >= 5:
                        self.status_text.set("注视右上")
                        self.send_hex_data_to_clients('aa551010')
                        print("注视右上")
                        self.enter_confirmation_state()
                elif avg_yaw > 5 and avg_pitch < 4:
                    if duration >= 1 and duration < 5:
                        self.status_text.set("观察右下")
                        if not self.observation_sent:
                            self.send_hex_data_to_clients('aa550111')
                            self.observation_sent = True
                    elif duration >= 5:
                        self.status_text.set("注视右下")
                        self.send_hex_data_to_clients('aa551011')
                        print("注视右下")
                        self.enter_confirmation_state()
                else:
                    self.status_text.set("选择中")
                    self.observation_sent = False  # 清除发送标志

                if current_time - self.start_time > 5:
                    self.start_time = current_time
                    self.yaw_buffer.clear()
                    self.pitch_buffer.clear()
                    self.roll_buffer.clear()
                    self.observation_sent = False  # 清除发送标志

            elif self.current_state == "确认中":
                if self.detect_shake():
                    print("检测到摇头")
                    self.return_to_selection_state()
                elif self.detect_nod():

                    self.status_text.set("进入第三个状态")
                    print("检测到点头")
                    print("1101")
                    
                    self.return_to_selection_state()
                elif current_time - self.confirm_start_time >= 10:
                    self.return_to_selection_state()

    def enter_confirmation_state(self):
        self.current_state = "确认中"
        self.confirm_start_time = time.time()
        self.status_text.set("确认中")

        # self.send_hex_data_to_clients('aa5510')

        self.yaw_buffer.clear()
        self.pitch_buffer.clear()
        self.roll_buffer.clear()
        self.nod_stage = 0
        self.shake_stage = 0
        self.observation_sent = False  # 清除发送标志

    def return_to_selection_state(self):
        print("22\n")
        # self.send_hex_data_to_clients('aa5522')
        self.current_state = "选择中"
        self.status_text.set("返回选择中")
        self.yaw_buffer.clear()
        self.pitch_buffer.clear()
        self.roll_buffer.clear()
        self.start_time = time.time()
        self.nod_stage = 0
        self.shake_stage = 0
        self.paused = True  # 设置暂停标志
        self.root.after(2500, self.unpause)  # 2秒后恢复运行
        self.observation_sent = False  # 清除发送标志

    def unpause(self):
        # self.send_hex_data_to_clients('aa5522')
        self.paused = False  # 恢复运行
        self.start_time = time.time()  # 重置计时器

    def reset_state(self):
        self.send_hex_data_to_clients('aa5522')
        self.current_state = "Initial"
        self.status_text.set("Initial")
        self.face_detected = False
        self.yaw_buffer.clear()
        self.pitch_buffer.clear()
        self.roll_buffer.clear()
        self.start_time = None
        self.nod_stage = 0
        self.shake_stage = 0
        self.observation_sent = False  # 清除发送标志
        print("State reset due to face disappearance.")

    def detect_nod(self):
        if len(self.pitch_buffer) > 5:
            max_pitch = max(self.pitch_buffer)
            current_pitch = self.pitch_buffer[-1]
            if self.nod_stage == 0:
                if (max_pitch - current_pitch > 15) and max_pitch > 0:
                    self.nod_stage = 1
            elif self.nod_stage == 1:
                min_pitch = min(self.pitch_buffer)
                if (min_pitch - current_pitch < -15) and min_pitch < 0:
                    self.send_hex_data_to_clients('aa551101')
                    return True
        return False

    def detect_shake(self):
        if len(self.yaw_buffer) > 5:
            max_yaw = max(self.yaw_buffer)
            current_yaw = self.yaw_buffer[-1]
            if self.shake_stage == 0:
                if (max_yaw - current_yaw > 45) and max_yaw > 30:
                    self.shake_stage = 1
            elif self.shake_stage == 1:
                min_yaw = min(self.yaw_buffer)
                if (min_yaw - current_yaw < -45) and min_yaw < -30:
                    return True
        return False

    def send_hex_data_to_clients(self, hex_string):
        hex_data = bytes.fromhex(hex_string)
        for client in self.clients:
            try:
                client.sendall(hex_data)
            except Exception as e:
                print(f"Error sending hex data to client: {e}")

    def display_image(self, image):
        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        image = Image.fromarray(image)
        image = ImageTk.PhotoImage(image)
        self.panel.configure(image=image)
        self.panel.image = image

    def on_closing(self):
        self.running = False
        self.server_socket.close()
        for client in self.clients:
            client.close()
        self.root.destroy()

if __name__ == "__main__":
    host = '0.0.0.0'
    port = 8888
    ImageServer(host, port).root.mainloop()
