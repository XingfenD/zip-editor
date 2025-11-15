#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
调试服务器脚本
用于接收来自DebugHelper类发送的调试数据
"""

import socket
import threading
import sys
import time
import argparse
from datetime import datetime


class DebugServer:
    def __init__(self, host='0.0.0.0', port=9000, buffer_size=4096):
        """
        初始化调试服务器

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
        self.client_count = 0  # 跟踪当前连接的客户端数量

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
            print(f"[*] 调试服务器已启动，监听 {self.host}:{self.port}")
            print(f"[*] 等待来自DebugHelper的连接...")
            print(f"[*] 按 Ctrl+C 停止服务器")

            # main loop, accept new connections
            while self.running:
                try:
                    # accept client connection
                    client_socket, client_address = self.server_socket.accept()

                    with self.lock:
                        self.client_count += 1
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

                # 可选：发送确认响应
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

    def stop(self):
        """停止调试服务器"""
        print("\n[*] 正在停止服务器...")
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
    """主函数"""
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