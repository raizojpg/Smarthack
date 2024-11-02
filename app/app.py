import requests

# Define the URL for the API endpoint
start_url = "http://localhost:8080/api/v1/session/start"
play_url = "http://localhost:8080/api/v1/play/round"
end_url = "http://localhost:8080/api/v1/session/end"

# Define the headers
headers = {
    "Content-Length": "0",
    "Accept": "*/*",
    "Accept-Encoding": "gzip, deflate, br",
    "Connection": "keep-alive",
    "API-KEY": "7bcd6334-bc2e-4cbf-b9d4-61cb9e868869"
}

def start_session():
    # Make the POST request
    response = requests.post(start_url, headers=headers)

    # Check if the request was successful
    if response.status_code == 200:
        try:

            # The response is a string with the session ID
            session_id = response.text.strip()
            print("Session started successfully. Session ID:", session_id)
            return session_id


        except ValueError:
            print("Error: Not OK")
    else:
        print(f"Failed to start session. Status Code: {response.status_code}")
        print("Response:", response.text)



def play_round(session_id, day, movements):
    # Define the request body
    body = {
        "day": day,
        "movements": movements
    }

    play_headers = headers.copy()
    play_headers["SESSION-ID"] = session_id

    # Make the POST request
    response = requests.post(play_url, headers=play_headers, json=body)

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


def end_session():
    # Make the POST request
    response = requests.post(end_url, headers=headers)

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


def main():
    session_id = start_session()
    print(session_id)

    if session_id:
        # Example movement
        movements = [
            {
                "connectionId": "3fa85f64-5717-4562-b3fc-2c963f66afa6",
                "amount": 0
            }
        ]
        play_round(session_id, 0, movements)

    end_session()


if __name__ == "__main__":
    main()