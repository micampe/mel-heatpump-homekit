#include <sys/pgmspace.h>

// generated file, edit web/index.html
const char* index_html PROGMEM = R"====(
<!DOCTYPE html>
<html lang='en'>
<head>
<title>__TITLE__</title>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'/>
<style>
body {
  text-align: center;
  font-family:sans-serif;
}

div, input {
  padding: 5px;
  font-size: 1em;
  margin: 5px 0;
  box-sizing: border-box;
}

input, button {
  border-radius: .3rem;
  width: 100%
}

button {
  cursor: pointer;
  border: 0;
  background-color: #1fa3ec;
  color: #fff;
  line-height: 2.4rem;
  font-size: 1.2rem;
  width: 100%
}

button:disabled {
  background-color: lightgray;
}

input[type='file'] {
  border:1px solid #1fa3ec
}

button.danger {
  background-color: #dc3630;
}

.wrap {
  text-align: left;
  display: inline-block;
  min-width: 260px;
  max-width: 400px
}

dl {
  width: 100%;
  overflow: hidden;
}

dt {
  display: inline-block;
  min-width: 100px;
  font-weight: bold;
}

dd {
  display: inline;
  margin: 0;
}

dd:after {
  display: block;
  content: '';
}

h2 {
  padding-top: 20px;
}

form {
  margin-bottom: 10px;
}

form#upload_form {
  margin-bottom: 40px;
}

.caption {
  font-size: small;
  color: darkgray;
}

.footer {
  margin-top: 40px;
  border-top: solid 1px darkgray;
  padding-top: 8px;
}

.footer a {
  font-size: smaller;
  color: darkgray;
}

@media (prefers-color-scheme: dark) {
  body {
    color: #ddd;
    background: #121212;
  }
  button:disabled {
    background-color: #333;
  }
}
</style>
</head>
<body>
<script>
function _(s) {
    if (s.charAt(0) === '#') {
        return document.querySelector(s)
    } else {
        return document.querySelectorAll(s)
    }
}

function rebootingTimer(button, msg) {
    let counter = 30
    button.disabled = true
    setInterval(function() {
        if (counter >= 0) {
            button.innerHTML = msg + counter + "s"
            counter -= 1
        } else {
            window.location.reload(1)
        }
    }, 1000);
}

function upload(e) {
    e.preventDefault()
    let form = _('#upload_form')
    let button = form.querySelector('button')
    button.innerHTML = 'Uploading…'
    button.disabled = true

    let formData = new FormData(form)
    let request = new XMLHttpRequest()
    request.onload = function(e) {
        let response = this.responseText
        if (response.includes('Update Success')) {
            rebootingTimer(button, 'Done, rebooting… ')
        } else {
            button.style = 'background-color:darkred'
            button.innerHTML = 'Error: ' + response.substr(response.indexOf(':') + 2)
        }
    }
    request.open('POST', form.action)
    request.send(formData)
}

function reboot(e) {
    e.preventDefault()
    let form = this
    let button = this.querySelector('button')
    let request = new XMLHttpRequest()
    request.onload = function (e) {
        if (request.status == 200) {
            rebootingTimer(button, 'Rebooting… ')
        }
    }
    request.open('POST', form.action)
    request.send()
}

function confirm(button) {
    var origContent = button.innerHTML
    var clickTime
    var tid
    var state = 0

    function cancel() {
        state = 0
        button.innerHTML = origContent
    }

    return function(event) {
        if (state == 0) {
            event.preventDefault()
            state = 1
            clickTime = +new Date()
            button.innerHTML = 'Are you sure?'
            tid = setTimeout(cancel, 3000)
        } else if (new Date() - clickTime < 500) {
            event.preventDefault()
        } else {
            clearTimeout(tid)
        }
    }
}

function saveSettings(e) {
    e.preventDefault()
    let formData = new FormData(this)
    let button = this.querySelector('button')
    let request = new XMLHttpRequest()
    request.onload = function(e) {
        if (request.status == 200) {
            rebootingTimer(button, 'Saved, rebooting…')
        }
    }
    request.open('POST', '/_settings')
    request.send(formData)
}

function loadSettings() {
    let request = new XMLHttpRequest()
    request.onload = function (ev) {
        let json = JSON.parse(request.response)
        for (let key in json) {
            let el = _('[name=' + key + ']')[0];
            if (el) {
                el.value = json[key]
            }
        }
    }
    request.open('GET', '/_settings')
    request.send()
}

window.onload = function () {
    _('#firmware_file').onchange = function (e) {
        _('#upload_button').disabled = this.files.lenght === 0
    }

    _('.needsConfirm').forEach(function (button) {
        button.onclick = confirm(button)
    })

    _('#upload_form').onsubmit = upload
    _('#reboot_form').onsubmit = reboot
    _('#unpair_form').onsubmit = reboot
    _('#settings_form').onsubmit = saveSettings

    loadSettings()
}
</script>
  <div class='wrap'>
    <h1>__TITLE__</h1>
    <dl>
    <dt>Heat Pump:</dt><dd>__HEAT_PUMP_STATUS__</dd>
    <dt>HomeKit:</dt><dd>__HOMEKIT_STATUS__</dd>
    <dt>Env Sensor:</dt><dd>__ENV_SENSOR_STATUS__</dd>
    <dt>MQTT:</dt><dd>__MQTT_STATUS__</dd>
    <dt>Uptime:</dt><dd>__UPTIME__</dd>
    <dt>Heap:</dt><dd>__HEAP__B</dd>
    <dt>Firmware:</dt><dd>__FIRMWARE_VERSION__</dd>
    </dl>
    <h2>Settings</h2>
    <form id='settings_form' action='/_settings' method='post'>
      <p><label for='mqtt_server'>MQTT Broker</label>
      <input placeholder='Address' name='mqtt_server' type='text' maxlength=32>
      <input placeholder='Port' value='1883' name='mqtt_port' type='text' maxlength=6>
      </p>
      <p><label for='mqtt_temp'>Environment Sensor</label>
      <input placeholder='Temperature reporting topic' name='mqtt_temp' type='text' maxlength=80>
      <input placeholder='Relative humidity reporting topic' name='mqtt_hum' type='text' maxlength=80>
      <span class='caption'>If these topics are set, temperature and relative
        humidity readings will be periodically posted to mqtt. &mdash; This
        feature requires an external sensor.</p>
  <!--
      <p><label for='mqtt_remote_temp'>Remote Temperature</label>
      <input placeholder='Remote temperature topic' name='mqtt_remote_temp' type='text' maxlength=80>
      <span class='caption'>An external room sensor can report the temperature for the heat pump
      thermostat.</p>
  -->
      <button>Save Settings</button>
      <p class='caption'>Saving settings will reboot the ESP.</p>
    </form>
    <h2>Firmware Update</h2>
    <form id='upload_form' action='/_update' method='post' enctype='multipart/form-data'>
      <input id='firmware_file' name='firmware' type='file' accept='.bin,.bin.gz,.gz'>
      <button id='upload_button' disabled>Update Firmware</button>
    </form>
    <form id='reboot_form' action='/_reboot' method='post'>
      <button class="needsConfirm">Reboot ESP</button>
    </form>
    <form id='unpair_form' action='/_unpair' method='post'>
      <button class="danger needsConfirm">Reset HomeKit Pairing</button>
    </form>
    <p class="footer">
    <a href="https://github.com/micampe/mel-heatpump-homekit">
      github.com/micampe/mel-heatpump-homekit</a>
    </p>
  </div>
</body>
</html>

)====";
