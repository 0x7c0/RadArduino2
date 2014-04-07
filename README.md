#RadArduino2


###use your arduino (due) like a radar
I attach on servos a big antenna for wifi detection, and with RaspberryPi I do many tests.<br>
There are some trouble with big servos (choose correct voltage).<br>
You can other info like temperature, light, lag, file on sd card.

_see project (hardware) with fritzing.fzz file @ http://fritzing.org/home/_

========

you must change arduino2 code with your prefer ip address (byte ip[], byte gateway[], ecc), use only a static address.<br>
Load all files inside SD directory in your Sd Card on Ethernet Shield.<br>
Afther that, you can access you Radar with any browser, for best user experience i suggest Chrome.

I use this setting:

* _created with:_ arduino ide 1.5.3
* _tested with:_ arduino ide 1.5.6r2
* 
* _created with:_ fritzing 0.8.4b
* _tested with:_ fritzing 0.8.7b
* 
* _viewed with:_ chrome on mac

<br>
_if you want change html file; remember to save html like htm (for fat limitation [.XXX]), and gzip css and js file (with gzip *.js/css)_
