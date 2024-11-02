import json
import os
import threading
import uuid
from random import random

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


def write_demands_to_file(data, i=0):
    lines = []
    for sub_dict in data['demand']:
        line = ""
        for key, value in sub_dict.items():
            # if key == 'postDay':
            #     continue
            line += str(value) + " "
        lines.append(line.strip())

    with open('give_demands/demand' + str(i) + '.txt', 'w') as f:
        f.write("\n".join(lines))


def file_to_json(file_path, i=0):
    demand = []
    result = {'result' : i}
    keys = ['customerId', 'amount', 'postDay', 'startDay', 'endDay']
    file = open(file_path, 'r')
    for line in file.readlines():
        values = line.split()
        data = dict(zip(keys, values))
        demand.append(data)
    result['demand'] = demand
    return result


def calculate_movement(day):
    # if day == 0:
    #
    #     return []

    movements = []

    file = open('return_demands/demand' + str(day) + '.txt', 'r')

    while not file:
        time.sleep(1)
        file = open('return_demands/demand' + str(day) + '.txt', 'r')

    for line in file.readlines():
        values = line.split()
        dic = {"connectionId": values[0], "amount": int(values[1])}
        movements.append(dic)

    # movements = [
    #     {
    #         "connectionId": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
    #         "amount": 0
    #     }
    # ]
    return movements

def main_cpp():
    os.startfile('maincpp.exe')

def generate_random_json_body(i, num_movements=1):

    if i == 0:
        return []
    day = i
    movements = []
    for _ in range(num_movements):
        movement = {
            "connectionId": str(uuid.uuid4()),  # Generate a random UUID
            "amount": random() * 1000
        }
        movements.append(movement)

    random_json_body = {
        "day": day,
        "movements": movements
    }

    return json.dumps(random_json_body, indent=2)


def main():
    sm = Sesion_Manager()
    session_id = sm.start_session()
    print(session_id)

    if session_id:
        for i in range(0, 42):
            # movements = calculate_movement(i)
            movements = [
                {
                    "connectionId": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
                    "amount": 0
                }
            ]
            # movements = generate_random_json_body(i, 1)

            data_json = sm.play_round(i, movements)
            write_demands_to_file(data_json, i)
            # file_to_json('give_demands/demand' + str(i) + '.txt', i)

    sm.end_session()


if __name__ == "__main__":

    thread = threading.Thread(target = main_cpp)
    thread.start()
    main()
    thread.join()
