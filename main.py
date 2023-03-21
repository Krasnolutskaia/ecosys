from flask import Flask, jsonify, render_template, url_for, redirect
from flask import request
import os

app = Flask(__name__)

status = {'rotate': 0, 'angle': 0, 'auto_rotate': 0, 'const_rotate': 0,
          'auto_light': 0, 'light': 0, 'light_border': 2000, 'day_duration': 0,
          'temp': -1.0, 'hum_air': -1.0, 'hum_soil': 0, 'hum_soil_border': 2000,
          'auto_water': 0, 'water': 0, 'water_volume': 0,
          'lights': [{'n': 1, 'val': 0}, {'n': 2, 'val': 0}, {'n': 3, 'val': 0}, {'n': 4, 'val': 0}, ], }


@app.route('/', methods=["POST", "GET"])
def index():
    print(status)
    return render_template('index.html', temp=status['temp'], hum_air=status['hum_air'], hum_soil=status['hum_soil'],
                           lights=status['lights'], water=status['water'], auto_light=status['auto_light'],
                           light=status['light'], auto_rotate=status['auto_rotate'], const_rotate=status['const_rotate'])


@app.route('/auto_water', methods=["POST"])
def auto_water():
    if request.json['dat']:
        status['auto_water'] = 1
    else:
        status['auto_water'] = 0
    return 'ok'


@app.route('/auto_light', methods=["POST"])
def auto_light():
    if request.json['dat']:
        status['auto_light'] = 1
    else:
        status['auto_light'] = 0
    return 'ok'


@app.route('/light', methods=["POST"])
def light():
    print(request.json['dat'])
    if request.json['dat']:
        status['light'] = 1
    else:
        status['light'] = 0
    return 'ok'


@app.route('/auto_rotate', methods=["POST"])
def auto_rotate():
    if request.json['dat']:
        status['auto_rotate'] = 1
    else:
        status['auto_rotate'] = 0
    return 'ok'


@app.route('/const_rotate', methods=["POST"])
def const_rotate():
    if request.json['dat']:
        status['const_rotate'] = 1
    else:
        status['const_rotate'] = 0
    return 'ok'


@app.route('/rotate', methods=["POST"])
def rotate():
    if request.form['angle']:
        status['rotate'] = 1
        status['angle'] = int(request.form['angle'])
    return redirect(url_for('index'))


@app.route('/water', methods=["POST"])
def water():
    if request.form['volume']:
        status['volume'] = int(request.form['volume'])
        status['water'] = 1
    return redirect(url_for('index'))


@app.route('/day_dur', methods=["POST"])
def day_dur():
    if request.form['day_dur']:
        status['day_duration'] = int(request.form['day_dur'])
    return redirect(url_for('index'))


@app.route('/from_greenhouse', methods=["POST"])
def from_greenhouse():
    if request.method == "POST":
        hum_air = request.form.get('hum_air')
        temp = request.form.get('temp')
        status['hum_soil'] = int(request.form.get('hum_soil'))
        status['rotate'] = int(request.form.get('rotate'))
        if status['auto_light']:
            status['light'] = int(request.form.get('light'))

        for i in range(4):
            status['lights'][i]['val'] = int(request.form.get(f'light_val{i}'))

        try:
            status['hum_air'] = float(hum_air)
            status['temp'] = float(temp)
        except:
            status['hum_air'] = 0
            status['temp'] = 0

        if hum_air == 'nan':
            status['hum_air'] = 0
        else:
            status['hum_air'] = float(hum_air)

        if temp == 'nan':
            status['temp'] = 0
        else:
            status['temp'] = float(temp)
    return 'ok'


@app.route('/to_greenhouse')
def to_greenhouse():
    return jsonify(status)


if __name__ == '__main__':
    port = int(os.environ.get("PORT", 80))
    app.run(debug=True, host='192.168.197.229', port=port)
