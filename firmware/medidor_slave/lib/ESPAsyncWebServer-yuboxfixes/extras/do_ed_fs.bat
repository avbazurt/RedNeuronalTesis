copy ..\src\edit.htm edit_src.htm
call html-minifier-terser --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes  --minify-css true --minify-js true -o edit.htm edit_src.htm
"C:\Program Files\7-Zip\7z.exe" a -tgzip -mx9 edit.htm.gz edit.htm
copy edit.htm.gz ..\examples\SmartSwitch\data\edit_gz
copy edit.htm.gz ..\examples\ESP_AsyncFSBrowser\data\edit_gz
ehg edit.htm.gz PROGMEM
copy edit.htm.gz.h ..\src\edit.htm.gz.h
pause
del edit.htm edit.htm.gz edit.htm.gz.h edit_src.htm