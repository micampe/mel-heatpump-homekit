import sys

Import('env')

if env['UPLOAD_PROTOCOL'] == 'custom':
    if 'OTA_ADDRESS' in env['ENV']:
        env.Replace(OTA_ADDRESS=env['ENV']['OTA_ADDRESS'])
        env.Replace(UPLOADCMD='curl -F "firmware=@$SOURCE" $OTA_ADDRESS/_update')
    else:
        raise RuntimeError('No OTA upload address specified, set OTA_ADDRESS variable')
