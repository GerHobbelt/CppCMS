#!/bin/sh
msgfmt --endianness=big he/LC_MESSAGES/simple.po -o he/LC_MESSAGES/simple.mo
msgfmt he/LC_MESSAGES/default.po -o he/LC_MESSAGES/default.mo
