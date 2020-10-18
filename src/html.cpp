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

.wrap {
  text-align: left;
  display: inline-block;
  min-width: 260px;
  max-width:500px
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

.footer {
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

function upload(e) {
    _('#upload_button').innerHTML = 'Uploading…'
    _('#upload_button').disabled = true

    let file = _('#firmware_file')
    let form = _('#upload_form')
    let formData = new FormData(form)
    let request = new XMLHttpRequest()
    request.open('POST', form.action, true)
    request.onload = function(e) {
        let response = this.responseText
        if (response.includes('Update Success')) {
            let counter = 30
            setInterval(function() {
                if (counter >= 0) {
                    _('#upload_button').innerHTML = 'Done, rebooting… '+ counter + "s"
                    counter -= 1
                } else {
                    window.location.reload(1)
                }
            }, 1000);
        } else {
            _('#upload_button').style = 'background-color:darkred'
            _('#upload_button').innerHTML = 'Error: ' + response.substr(response.indexOf(':') + 2)
        }
    }
    request.send(formData)
    return false
}

window.onload = function () {
    _('#firmware_file').onchange = function (e) {
        _('#upload_button').disabled = this.files.lenght === 0
    }

    _('#upload_button').onclick = upload
}
</script>
<body>
  <div class='wrap'>
    <h1>__TITLE__</h1>
    <dl>
    <dt>Uptime:</dt><dd>__UPTIME__</dd>
    <dt>Heap:</dt><dd>__HEAP__B</dd>
    <dt>Firmware:</dt><dd>__FIRMWARE_VERSION__</dd>
    </dl>
    <form id='upload_form' action='/_update' method='post' enctype='multipart/form-data'>
      <input id='firmware_file' name='firmware' type='file' accept='.bin,.bin.gz,.gz'>
      <button id='upload_button' type='submit' disabled>Update Firmware</button>
    </form>
    <p class="footer">
    <a href="https://github.com/micampe/mel-heatpump-homekit">
      github.com/micampe/mel-heatpump-homekit</a>
    </p>
  </div>
</body>
</html>

)====";
