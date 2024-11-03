import json
import os
import threading
import uuid
from random import random
import subprocess
import xdg

import requests
import time

class Sesion_Manager:
    def __init__(self):
        self.start_url = "http://localhost:8080/api/v1/session/start"
        self.play_url = "http://localhost:8080/api/v1/play/round"
        self.end_url = "http://localhost:8080/api/v1/session/end"
        self.headers = {
            "Content-Length": "0",
            "Accept": "*/*",
            "Accept-Encoding": "gzip, deflate, br",
            "Connection": "keep-alive",
            "API-KEY": "7bcd6334-bc2e-4cbf-b9d4-61cb9e868869"
        }
        self.session_id = None

    def start_session(self):
        # Make the POST request
        response = requests.post(self.start_url, headers=self.headers)

        # Check if the request was successful
        if response.status_code == 200:
            try:

                # The response is a string with the session ID
                self.session_id = response.text.strip()
                print("Session started successfully. Session ID:", self.session_id)
                return self.session_id

            except ValueError:
                print("Error: Not OK")
        else:
            print(f"Failed to start session. Status Code: {response.status_code}")
            print("Response:", response.text)

    def play_round(self, day, movements):
        # Define the request body
        body = {
            "day": day,
            "movements": movements
        }

        play_headers = self.headers.copy()
        play_headers["SESSION-ID"] = self.session_id

        # Make the POST request
        response = requests.post(self.play_url, headers=play_headers, json=body)

        # Check if the request was successful
        if response.status_code == 200:
            try:
                # the response is a JSON object
                json_response = response.json()
                print("Round played successfully. Response:", json_response)
                return json_response
            except ValueError:
                print("Error: Not OK")
        else:
            print(f"Failed to play round. Status Code: {response.status_code}")
            print("Response:", response.text)

    def end_session(self):
        # Make the POST request
        response = requests.post(self.end_url, headers=self.headers)

        if response.status_code == 200:
            try:
                # Parse the JSON response
                json_response = response.json()
                print("Session ended successfully. Response:", json_response)
                return json_response
            except ValueError:
                print("Error: Not OK")
        else:
            print(f"Failed to end session. Status Code: {response.status_code}")
            print("Response:", response.text)

def write_demands_to_file(data, i=1):
    lines = []
    for sub_dict in data['demand']:
        line = ""
        for key, value in sub_dict.items():
            # if key == 'postDay':
            #     continue
            line += str(value) + " "
        lines.append(line.strip())

    with open('result_demands/demand' + str(i) + '.txt', 'w') as f:
        f.write("\n".join(lines))

def calculate_movement(sm, day):
    if day == 0:
        return []

    movements = []

    print('give_demands/demand' + str(day) + '.txt')
    filename = 'give_demands/demand' + str(day) + '.txt';

    while True:
        if os.path.isfile(filename):
            with open(filename, 'r') as file:
                for line in file.readlines():
                    values = line.split()
                    dic = {"connectionId": values[0], "amount": int(values[1])}
                    movements.append(dic)
            break
        else:
            print(day)
            time.sleep(0.5)

    return movements

def main_cpp():
    subprocess.run(['./main'], capture_output=True, text=True)

def main():
    sm = Sesion_Manager()
    session_id = sm.start_session()
    print(session_id)

    movements = []
    if session_id:
        for i in range(0, 42):
            data_json = sm.play_round(i, movements)
            write_demands_to_file(data_json, i)
            movements = calculate_movement(sm, i)

    sm.end_session()

if __name__ == "__main__":

    thread = threading.Thread(target = main_cpp)
    thread.start()
    main()
    thread.join()
