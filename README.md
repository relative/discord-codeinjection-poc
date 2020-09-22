poc for codeinjection for discord/electron based Win32 apps

you will likely have to change the include path for node

```bash
git clone https://github.com/nodejs/node --recursive
cd node
git checkout v12.14.1 # DiscordCanary's node version
```

and add to VC++ Directories in VS / edit the vcxproj

# node

calling napi_run_script will not be in the module scope, so you will not be able to use require
