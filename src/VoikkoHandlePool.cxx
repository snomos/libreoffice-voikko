/* Libreoffice-voikko: Finnish linguistic extension for LibreOffice
 * Copyright (C) 2010 - 2011 Harri Pitkänen <hatapitk@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/

#include "VoikkoHandlePool.hxx"
#include "common.hxx"
#include "macros.hxx"

namespace voikko {

using namespace std;
using namespace ::rtl;
using namespace ::com::sun::star;

struct Bcp47ToOOoMapping {
	const char * bcpTag;
	const char * oooLanguage;
	const char * oooRegion;
};

static Bcp47ToOOoMapping const bcpToOOoMapping[] = {
	{"af", "af", "NA"},
	{"af", "af", "ZA"},
	{"am", "am", "ET"},
	{"ar", "ar", "AE"},
	{"ar", "ar", "BH"},
	{"ar", "ar", "DJ"},
	{"ar", "ar", "DZ"},
	{"ar", "ar", "EG"},
	{"ar", "ar", "ER"},
	{"ar", "ar", "IL"},
	{"ar", "ar", "IQ"},
	{"ar", "ar", "JO"},
	{"ar", "ar", "KM"},
	{"ar", "ar", "KW"},
	{"ar", "ar", "LB"},
	{"ar", "ar", "LY"},
	{"ar", "ar", "MA"},
	{"ar", "ar", "MR"},
	{"ar", "ar", "OM"},
	{"ar", "ar", "PS"},
	{"ar", "ar", "QA"},
	{"ar", "ar", "SA"},
	{"ar", "ar", "SD"},
	{"ar", "ar", "SO"},
	{"ar", "ar", "SY"},
	{"ar", "ar", "TD"},
	{"ar", "ar", "TN"},
	{"ar", "ar", "YE"},
	{"bn", "bn", "BD"},
	{"bn", "bn", "IN"},
	{"ca", "ca", "AD"},
	{"ca", "ca", "ES"},
	{"cs", "cs", "CZ"},
	{"csb", "csb", "PL"},
	{"cy", "cy", "GB"},
	{"de", "de", "DE"},
	{"de-AT", "de", "AT"},
	{"de-BE", "de", "BE"},
	{"de-CH", "de", "CH"},
	{"de-LU", "de", "LU"},
	{"el", "el", "GR"},
	{"en-US", "en", "US"},
	{"en-US", "en", "ZA"},
	{"eo", "eo", ""},
	{"es", "es", "ES"},
	{"es-MX", "es", "MX"},
	{"et", "et", "EE"},
	{"fa", "fa", "IR"},
	{"fi", "fi", "FI"},
	{"fo", "fo", "FO"},
	{"fr", "fr", "FR"},
	{"fr-BE", "fr", "BE"},
	{"fr-CA", "fr", "CA"},
	{"fr-CH", "fr", "CH"},
	{"fr-LU", "fr", "LU"},
	{"fr-MC", "fr", "MC"},
	{"kl", "kl", "GL"},
	{"se", "se", "FI"},
	{"se", "se", "NO"},
	{"se", "se", "SE"},
	{0, 0, 0}
};

VoikkoHandlePool::VoikkoHandlePool() {
	for (const Bcp47ToOOoMapping * m = bcpToOOoMapping; m->bcpTag; ++m) {
		bcpToOOoMap.insert(pair<string, pair<string, string> >(m->bcpTag,
		                   pair<string, string>(m->oooLanguage, m->oooRegion)));
	}
}

VoikkoHandlePool * VoikkoHandlePool::getInstance() {
	if (!instance) {
		instance = new VoikkoHandlePool();
	}
	return instance;
}

const char * VoikkoHandlePool::getInstallationPath() {
	if (installationPath.getLength() == 0) {
		return 0;
	} else {
		return installationPath.getStr();
	}
}

VoikkoHandle * VoikkoHandlePool::openHandleWithVariant(const OString & language, const OString & fullVariant) {
	const char * errorString = 0;
	VOIKKO_DEBUG("VoikkoHandlePool::openHandleWithVariant");
	VoikkoHandle * voikkoHandle = voikkoInit(&errorString, fullVariant.getStr(), getInstallationPath());
	if (voikkoHandle) {
		handles[language] = voikkoHandle;
		for (map<int, bool>::const_iterator it = globalBooleanOptions.begin(); it != globalBooleanOptions.end(); it++) {
			voikkoSetBooleanOption(voikkoHandle, it->first, it->second ? 1 : 0);
		}
		for (map<int, int>::const_iterator it = globalIntegerOptions.begin(); it != globalIntegerOptions.end(); it++) {
			voikkoSetIntegerOption(voikkoHandle, it->first, it->second);
		}
		return voikkoHandle;
	} else {
		initializationErrors[language] = errorString;
		return 0;
	}
}

VoikkoHandle * VoikkoHandlePool::openHandle(const OString & language) {
	if (getPreferredGlobalVariant().getLength() > 0) {
		OString languageWithVariant = language + OString("-x-") +
			OUStringToOString(getPreferredGlobalVariant(), RTL_TEXTENCODING_UTF8);
		VoikkoHandle * handle = openHandleWithVariant(language, languageWithVariant);
		if (handle) {
			return handle;
		}
	}
	return openHandleWithVariant(language, language);
}

VoikkoHandle * VoikkoHandlePool::getHandle(const lang::Locale & locale) {
	OString language = OUStringToOString(locale.Language, RTL_TEXTENCODING_UTF8);
	if (handles.find(language) != handles.end()) {
		return handles[language];
	}
	if (initializationErrors.find(language) != initializationErrors.end()) {
		return 0;
	}
	return openHandle(language);
}

void VoikkoHandlePool::closeAllHandles() {
	for (map<OString,VoikkoHandle *>::const_iterator it = handles.begin(); it != handles.end(); it++) {
		voikkoTerminate(it->second);
	}
	handles.clear();
	initializationErrors.clear();
}

void VoikkoHandlePool::setGlobalBooleanOption(int option, bool value) {
	if (globalBooleanOptions.find(option) != globalBooleanOptions.end() && globalBooleanOptions[option] == value) {
		return;
	}
	globalBooleanOptions[option] = value;
	for (map<OString,VoikkoHandle *>::const_iterator it = handles.begin(); it != handles.end(); it++) {
		voikkoSetBooleanOption(it->second, option, value ? 1 : 0);
	}
}

void VoikkoHandlePool::setGlobalIntegerOption(int option, int value) {
	if (globalIntegerOptions.find(option) != globalIntegerOptions.end() && globalIntegerOptions[option] == value) {
		return;
	}
	globalIntegerOptions[option] = value;
	for (map<OString,VoikkoHandle *>::const_iterator it = handles.begin(); it != handles.end(); it++) {
		voikkoSetIntegerOption(it->second, option, value);
	}
}

void VoikkoHandlePool::addLocale(uno::Sequence<lang::Locale> & locales, const char * language) {
	// TODO: n^2 performance because sequence is reconstructed on every add
	sal_Int32 position = locales.getLength();
	pair<multimap<string, pair<string, string> >::iterator, multimap<string, pair<string, string> >::iterator>
	   matchingLangs = bcpToOOoMap.equal_range(language);
	for (multimap<string, pair<string, string> >::iterator it = matchingLangs.first;
	     it != matchingLangs.second; ++it) {
		locales.realloc(++position);
		OUString lang = A2OU((*it).second.first.c_str());
		OUString region = A2OU((*it).second.second.c_str());
		locales.getArray()[position - 1] = lang::Locale(lang, region, OUString());
	}
}

uno::Sequence<lang::Locale> VoikkoHandlePool::getSupportedSpellingLocales() {
	char ** languages = voikkoListSupportedSpellingLanguages(getInstallationPath());
	uno::Sequence<lang::Locale> locales(0);
	for (char ** i = languages; *i; i++) {
		addLocale(locales, *i);
	}
	voikkoFreeCstrArray(languages);
	return locales;
}

uno::Sequence<lang::Locale> VoikkoHandlePool::getSupportedHyphenationLocales() {
	uno::Sequence<lang::Locale> spellingLocales = getSupportedSpellingLocales();
	for (sal_Int32 i = 0; i < spellingLocales.getLength(); i++) {
		if (spellingLocales[i].Language == A2OU("fi")) {
			uno::Sequence<lang::Locale> locales(1);
			locales.getArray()[0] = lang::Locale(A2OU("fi"), A2OU("FI"), OUString());
			return locales;
		}
	}
	return uno::Sequence<lang::Locale>(0);
}

uno::Sequence<lang::Locale> VoikkoHandlePool::getSupportedGrammarLocales() {
	return getSupportedHyphenationLocales();
}

rtl::OUString VoikkoHandlePool::getInitializationStatus() {
	// FIXME: more informative status message
	if (handles.empty()) {
		return A2OU("Not initialized");
	}
	return A2OU("OK");
}

void VoikkoHandlePool::setPreferredGlobalVariant(const OUString & variant) {
	if (variant != this->preferredGlobalVariant) {
		this->preferredGlobalVariant = variant;
		closeAllHandles();
	}
}

void VoikkoHandlePool::setInstallationPath(const rtl::OString & path) {
	this->installationPath = path;
}

OUString VoikkoHandlePool::getPreferredGlobalVariant() {
	return this->preferredGlobalVariant;
}

static bool containsLocale(const lang::Locale & localeToFind, const uno::Sequence<lang::Locale> & locales) {
	for (sal_Int32 i = 0; i < locales.getLength(); i++) {
		if (locales[i].Language == localeToFind.Language &&
		    locales[i].Country == localeToFind.Country) {
			return true;
		}
	}
	return false;
}

bool VoikkoHandlePool::supportsSpellingLocale(const lang::Locale & locale) {
	return containsLocale(locale, getSupportedSpellingLocales());
}

bool VoikkoHandlePool::supportsHyphenationLocale(const lang::Locale & locale) {
	return containsLocale(locale, getSupportedHyphenationLocales());
}

bool VoikkoHandlePool::supportsGrammarLocale(const lang::Locale & locale) {
	return containsLocale(locale, getSupportedGrammarLocales());
}

VoikkoHandlePool * VoikkoHandlePool::instance = 0;

}