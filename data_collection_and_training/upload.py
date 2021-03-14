import json
import time
import hmac
import hashlib
import os

HMAC_KEY = "<insert your edge impulse hmac key>"
labels = {
    '1': 'Person',
    '2': 'Object',
    '3': 'Background'
}

dir = 'raw_data'
for filename in os.listdir(dir):
    if filename.endswith('.csv'):
        prefix, ext = os.path.splitext(filename)
        label = labels[prefix[-1]]
        outfilename = os.path.join('formatted_data', '{}.{}.json'.format(label, prefix))
        with open(os.path.join(dir, filename)) as fp:
            values = [[float(i)] for i in fp.read().split(',')]
            emptySignature = ''.join(['0'] * 64)
            data = {
                "protected": {
                    "ver": "v1",
                    "alg": "HS256",
                    "iat": time.time()
                },
                "signature": emptySignature,
                "payload": {
                    "device_name": "DI:C0:F3:08:33:96",
                    "device_type": "Raspberry_Pi_Pico",
                    "interval_ms": 1,
                    "sensors": [ 
                        { "name": "temperature", "units": "Cel" },
                    ],
                    "values": values
                }
            }

            # encode in JSON
            encoded = json.dumps(data)

            # sign message
            signature = hmac.new(bytes(HMAC_KEY, 'utf-8'), msg = encoded.encode('utf-8'), digestmod = hashlib.sha256).hexdigest()

            # set the signature again in the message, and encode again
            data['signature'] = signature
            encoded = json.dumps(data, indent=4)
            with open(outfilename, 'w') as fout:
                fout.write(encoded)

