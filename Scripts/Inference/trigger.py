
from mprpc import RPCClient
import requests
import sys

OBJECT_LIST = [ 'cat', 'soccer', 'helmet', 'basketball' ]
PREDICTION_URL = 'http://127.0.0.1/predictions/onnx-inception-v1'
HOST = '127.0.0.1'
PORT = 8080
METHOD_CREATE_OBJECT = 'OnObjectInference'

def get_object_name(prob_list: list):
    for prob in prob_list:
        prob_class = prob['class']
        for obj_name in OBJECT_LIST:
            if obj_name in prob_class:
                return obj_name
    
    return None

def predict(image_file: str):
    response = requests.post(PREDICTION_URL, data=open(image_file, 'rb')) 
    obj_name = get_object_name(response.json())
    return obj_name

def trigger_object_creation(object_name: str):
    client = RPCClient(HOST, PORT)
    client.call(METHOD_CREATE_OBJECT, object_name)


def main():
    image_file = sys.argv[1]
    object_name = predict(image_file)
    if object_name is not None:
        print('Object: ' + object_name)
        trigger_object_creation(object_name)
    else:
        print('Could not infer object')

if __name__ == "__main__":
    main()