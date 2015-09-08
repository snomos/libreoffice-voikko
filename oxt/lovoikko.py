# Libreoffice-voikko: Linguistic extension for LibreOffice
# Copyright (C) 2015 Harri Pitkänen <hatapitk@iki.fi>
#
# This Source Code Form is subject to the terms of the Mozilla Public License,
# v. 2.0. If a copy of the MPL was not distributed with this file, You can
# obtain one at http://mozilla.org/MPL/2.0/.
# 
# Alternatively, the contents of this file may be used under the terms of
# the GNU General Public License Version 3 or later (the "GPL"), in which
# case the provisions of the GPL are applicable instead of those above.

import unohelper
from SettingsEventHandler import SettingsEventHandler
from SpellChecker import SpellChecker

# name of g_ImplementationHelper is significant, Python component loader expects to find it
g_ImplementationHelper = unohelper.ImplementationHelper()
g_ImplementationHelper.addImplementation(SettingsEventHandler, \
                    "org.puimula.ooovoikko.SettingsEventHandlerImplementation",
                    ("org.puimula.ooovoikko.SettingsEventHandlerService",),)
g_ImplementationHelper.addImplementation(SpellChecker, \
                    "voikko.SpellChecker",
                    ("com.sun.star.linguistic2.SpellChecker",),)