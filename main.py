import json
import requests
from flask import Flask, jsonify, render_template, url_for, redirect
from flask import request
from datetime import datetime, timedelta
import os

app = Flask(__name__)

status = {'rotate': 0, 'angle': 0, 'auto_rotate': 0, 'const_rotate': 0,
          'auto_light': 0, 'light': 0, 'light_border': 2000, 'day_duration': 0, 'light_lvl': 0,
          'last_checked': datetime.fromtimestamp(1000000000), 'sunrise_h': 0, 'sunset_h': 0, 'timezone': 0, 'l_tz': 0,
          'sunrise_str': '', 'sunset_str': '', 'city': 'Moscow', 'curr_h': 0,
          'temp': -1.0, 'hum_air': -1.0, 'hum_soil': 0, 'hum_soil_border': 2000,
          'auto_water': 0, 'water': 0, 'water_volume': 0,
          'lights': [{'n': 1, 'val': 0}, {'n': 2, 'val': 0}, {'n': 3, 'val': 0}, {'n': 4, 'val': 0}, ], }


@app.route('/', methods=["POST", "GET"])
def index():
    print(status)
    return render_template('index.html', temp=status['temp'], hum_air=status['hum_air'], hum_soil=status['hum_soil'],
                           light_lvl=status['light_lvl'], lights=status['lights'], city=status['city'])


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


@app.route('/set_city', methods=["POST"])
def set_city():
    if request.form['set_city']:
        status['city'] = request.form['set_city']
        get_sun_info()
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

        light_lvl = 0
        for i in range(4):
            val = int(request.form.get(f'light_val{i}'))
            status['lights'][i]['val'] = val
            light_lvl += val
        status['light_lvl'] = 100 - int(light_lvl / 16384 * 100)

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


@app.route('/get_data')
def get_data():
    if next_day():
        get_sun_info()
    get_curr_hour()
    return jsonify(status)


def get_curr_hour():
    td = timedelta(seconds=status['timezone'])
    dt_now = datetime.utcnow() + td
    status['curr_h'] = dt_now.hour


def get_sun_info():
    r = requests.get(f"https://api.openweathermap.org/data/2.5/weather?q={status['city']}&appid=61353381666a7580e62eb549c0b7b099")
    time.sleep(1)
    r = json.loads(r.text)
    if r["cod"] == 200:
        status['last_checked'] = datetime.utcnow()
        dt = datetime.now() - datetime.utcnow()
        status['l_tz'] = dt.seconds
        status['timezone'] = int(r["timezone"])
        ss = datetime.fromtimestamp(int(r["sys"]["sunset"])) - datetime.fromtimestamp(status['l_tz']) + datetime.fromtimestamp(int(r["timezone"]))
        sr = datetime.fromtimestamp(int(r["sys"]["sunrise"])) - datetime.fromtimestamp(status['l_tz']) + datetime.fromtimestamp(int(r["timezone"]))
        status['sunset_str'], status['sunrise_str'] = ss.strftime('%H:%M'), sr.strftime('%H:%M')
        status['sunset_h'], status['sunrise_h'] = ss.hour, sr.hour


def next_day():
    dt_now = datetime.utcnow()
    dt_lc = status['last_checked']
    if dt_now.year - dt_lc.year > 0 or dt_now.month - dt_lc.month > 0 or dt_now.day - dt_lc.day > 0:
        return True
    return False


if __name__ == '__main__':
    port = int(os.environ.get("PORT", 80))
    app.run(debug=True, host='172.16.102.41', port=port)
