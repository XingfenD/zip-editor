#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Debug server script
Used to receive debug data sent from DebugHelper class
"""

import socket
import threading
import sys
import time
import argparse
import os
from datetime import datetime


class DebugServer:
    def __init__(self, host='0.0.0.0', port=9000, buffer_size=4096):
        """
        Initialize debug server

        Args:
            host (str): server listen address, default is 0.0.0.0 (all interfaces)
            port (int): server listen port, default is 9000
            buffer_size (int): receive buffer size
        """
        self.host = host
        self.port = port
        self.buffer_size = buffer_size
        self.server_socket = None
        self.running = False
        self.threads = []
        self.lock = threading.Lock()
        self.client_count = 0  # current connected client count
        self.start_time = None  # server start time
        self.total_messages = 0  # total message count
        self.total_clients = 0  # total connected client count

    def start(self):
        """start debug server"""
        try:
            # create server socket
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            # allow port reuse
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

            # bind address and port
            self.server_socket.bind((self.host, self.port))

            # start listening, max connection number is 5
            self.server_socket.listen(5)

            self.running = True
            self.start_time = datetime.now()

            # start command thread
            command_thread = threading.Thread(target=self.handle_commands)
            command_thread.daemon = True
            command_thread.start()

            print(f"[*] 调试服务器已启动，监听 {self.host}:{self.port}")
            print(f"[*] 等待来自DebugHelper的连接...")
            print(f"[*] 输入 'help' 查看可用命令")

            # set socket timeout, avoid accept() block forever
            self.server_socket.settimeout(1.0)  # 1 second timeout

            # main loop, accept new connections
            while self.running:
                try:
                    # accept client connection
                    client_socket, client_address = self.server_socket.accept()

                    with self.lock:
                        self.client_count += 1
                        self.total_clients += 1
                        print(f"\n[+] 新连接: {client_address[0]}:{client_address[1]} - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

                    # create a new thread to handle each client
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(client_socket, client_address)
                    )
                    client_thread.daemon = True
                    client_thread.start()

                    with self.lock:
                        self.threads.append(client_thread)

                except socket.timeout:
                    # timeout, continue loop
                    continue
                except socket.error as e:
                    # if server is not running, break loop
                    if not self.running:
                        break
                    print(f"[!] 接受连接时出错: {e}")

        except Exception as e:
            print(f"[!] 服务器启动失败: {e}")
            self.stop()
            raise

    def handle_client(self, client_socket, client_address):
        """handle client connection

        Args:
            client_socket
            client_address
        """
        try:
            while self.running:
                # receive data from client
                data = client_socket.recv(self.buffer_size)

                if not data:
                    # client closed connection
                    with self.lock:
                        self.client_count = max(0, self.client_count - 1)
                        print(f"[-] 客户端 {client_address[0]}:{client_address[1]} 断开连接")
                    break

                # decode data from client
                try:
                    message = data.decode('utf-8')
                except UnicodeDecodeError:
                    # if cannot decode as UTF-8, try other encoding or show raw data
                    message = f"[无法解码的二进制数据: {len(data)} 字节]"

                # display received message
                with self.lock:
                    timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                    # when only one client connected, do not show client address
                    if self.client_count == 1:
                        print(f"[{timestamp}]:")
                    else:
                        print(f"[{timestamp}] 来自 {client_address[0]}:{client_address[1]}:")
                    print(f"  {message}")
                    self.total_messages += 1

                # optional: send confirmation response to client
                # client_socket.send(b"Message received\n")

        except ConnectionResetError:
            with self.lock:
                self.client_count = max(0, self.client_count - 1)
                print(f"[-] 客户端 {client_address[0]}:{client_address[1]} 断开连接（连接被重置）")
        except Exception as e:
            with self.lock:
                self.client_count = max(0, self.client_count - 1)
                print(f"[!] 处理客户端 {client_address[0]}:{client_address[1]} 时出错: {e}")
        finally:
            # close client socket
            try:
                client_socket.close()
            except:
                pass
            # when client closed connection, client_count will be updated in above code

    def handle_commands(self):
        """handle server commands input"""
        while self.running:
            try:
                command = sys.stdin.readline().strip().lower()

                if not self.running:
                    break

                if command == 'help':
                    self.show_help()
                elif command == 'status':
                    self.show_status()
                elif command == 'clear':
                    self.clear_screen()
                elif command == 'exit' or command == 'quit':
                    self.running = False
                else:
                    if command:
                        print(f"[!] 未知命令: '{command}'. 输入 'help' 查看可用命令")
            except Exception as e:
                if self.running:
                    print(f"[!] 命令处理错误: {e}")
                break

    def show_help(self):
        """show help informations"""
        help_text = """
[*] 可用命令:
    help    - 显示此帮助信息
    status  - 显示服务器状态信息
    clear   - 清空屏幕
    exit    - 停止服务器并退出
        """
        print(help_text)

    def show_status(self):
        """显示服务器状态信息"""
        with self.lock:
            uptime = datetime.now() - self.start_time
            uptime_str = str(uptime).split('.')[0]

            status_text = f"""
[*] 服务器状态:
    监听地址: {self.host}:{self.port}
    运行时间: {uptime_str}
    当前连接: {self.client_count} 个客户端
    总连接数: {self.total_clients} 个客户端
    总消息数: {self.total_messages} 条消息
            """
            print(status_text)

    def clear_screen(self):
        """clear screen"""
        # clear screen
        os.system('cls' if os.name == 'nt' else 'clear')
        print(f"[*] 调试服务器 (运行中) - 输入 'help' 查看可用命令")

    def stop(self):
        """stop debug server"""
        print("\n[*] 正在停止服务器...")
        # set running to False to stop server loops
        self.running = False

        # close server socket
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass

        # wait for all threads to finish
        with self.lock:
            for thread in self.threads:
                if thread.is_alive():
                    thread.join(1)  # wait for thread to finish, max 1 second

        print("[*] 服务器已停止")


def main():
    # parse command line arguments
    parser = argparse.ArgumentParser(description='调试服务器 - 接收来自C++ DebugHelper的数据')
    parser.add_argument('-p', '--port', type=int, default=9000, help='服务器监听端口')
    parser.add_argument('-H', '--host', type=str, default='0.0.0.0', help='服务器监听地址')

    args = parser.parse_args()

    # create and start server
    server = DebugServer(host=args.host, port=args.port)

    try:
        server.start()
    except KeyboardInterrupt:
        print("\n[*] 接收到中断信号")
    finally:
        server.stop()


if __name__ == '__main__':
    main()