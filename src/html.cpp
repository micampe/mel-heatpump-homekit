#include <sys/pgmspace.h>

// generated file, edit web/index.html
const char* index_html PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<title>__TITLE__</title>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>
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

button, input[type='button'], input[type='submit'] {
  cursor: pointer;
  border: 0;
  background-color: #1fa3ec;
  color: #fff;
  line-height: 2.4rem;
  font-size: 1.2rem;
  width: 100%
}

button:disabled, input[type='button']:disabled, input[type='submit']:disabled {
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

form {
  margin-bottom: 10px;
}

form#upload_form {
  margin-bottom: 40px;
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
</style>
</head>
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
}
</script>
<body>
  <div class='wrap'>
    <h1>__TITLE__</h1>
    <dl>
    <dt>Heat Pump:</dt><dd>__HEAT_PUMP_STATUS__</dd>
    <dt>Uptime:</dt><dd>__UPTIME__</dd>
    <dt>Heap:</dt><dd>__HEAP__B</dd>
    <dt>Firmware:</dt><dd>__FIRMWARE_VERSION__</dd>
    </dl>
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
