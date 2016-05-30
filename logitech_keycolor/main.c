//
//  main.c
//  logitech_keycolor
//
//  Author: Karl Bunch <karlbunch@karlbunch.com>
//
//  Created: Sat May 28 17:07:16 EDT 2016
//
//  Copyright © 2016 Karl Bunch <http://www.karlbunch.com/>
//
//  The MIT License (MIT)
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//

#define VERSION "0.0.2"
#define COPYRIGHT "Copyright © 2016 Karl Bunch <http://www.karlbunch.com/>"

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <libgen.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

static CFMutableDictionaryRef setMatchSelection(CFMutableDictionaryRef dictRef, CFStringRef key, UInt32 value)
{
    if (dictRef == NULL) {
        dictRef = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        CFAutorelease(dictRef);
    
        if (!dictRef) {
            fprintf(stderr, "Failed to alloc matching dictionary!\n");
            exit(3);
        }
    }
    
    if (dictRef) {
        CFNumberRef tCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);
        CFDictionarySetValue(dictRef, key, tCFNumberRef);
        CFRelease(tCFNumberRef);
    }
    
    return dictRef;
}

void parseColor(char *str, uint8_t *red, uint8_t *green, uint8_t *blue)
{
    uint8_t *cc[] = { red, green, blue };
    int cIdx = 0;
    
    for (char *cp = strtok(str, ","); cp && cIdx < 3; cp = strtok(NULL, ",")) {
        *cc[cIdx++] = (uint8_t)(atoi(cp));
    }
    
    while (cIdx < 3) {
        *cc[cIdx] = 0;
    }
}

void usage(int exitCode, char * argv[], struct option *opts)
{
    char *me = basename(argv[0]);
    
    fprintf(stderr, "%s %s -- %s\n\nUsage: %s", me, VERSION, COPYRIGHT, me);
    
    for (;opts->name; opts++) {
        char *arg = "";
        
        if (opts->has_arg == required_argument) {
            switch(opts->val) {
            case 'c': arg = " {0-255}"; break;
            case 'C': arg = " {0-255,0-255,0-255}"; break;
            default: arg = " {arg}"; break;
            }
        }
        fprintf(stderr, " [--%s|-%c%s]", opts->name, opts->val, arg);
    }
    fprintf(stderr, "\n\n");
    exit(exitCode);
}

#define GUESS_UNKNOWN_TYPES 0
void cfValueToString(CFMutableStringRef str, CFTypeRef value)
{
#if GUESS_UNKNOWN_TYPES
    struct idMap {
        const char *name;
        CFTypeID id;
    } typeMap[] = {
        "CFAllocatorGetTypeID", CFAllocatorGetTypeID(),
        "CFArrayGetTypeID", CFArrayGetTypeID(),
        "CFAttributedStringGetTypeID", CFAttributedStringGetTypeID(),
        "CFBagGetTypeID", CFBagGetTypeID(),
        "CFBinaryHeapGetTypeID", CFBinaryHeapGetTypeID(),
        "CFBitVectorGetTypeID", CFBitVectorGetTypeID(),
        "CFBooleanGetTypeID", CFBooleanGetTypeID(),
        "CFBundleGetTypeID", CFBundleGetTypeID(),
        "CFCalendarGetTypeID", CFCalendarGetTypeID(),
        "CFCharacterSetGetTypeID", CFCharacterSetGetTypeID(),
        "CFDataGetTypeID", CFDataGetTypeID(),
        "CFDateFormatterGetTypeID", CFDateFormatterGetTypeID(),
        "CFDateGetTypeID", CFDateGetTypeID(),
        "CFDictionaryGetTypeID", CFDictionaryGetTypeID(),
        "CFErrorGetTypeID", CFErrorGetTypeID(),
        "CFFileDescriptorGetTypeID", CFFileDescriptorGetTypeID(),
        "CFFileSecurityGetTypeID", CFFileSecurityGetTypeID(),
        "CFLocaleGetTypeID", CFLocaleGetTypeID(),
        "CFMachPortGetTypeID", CFMachPortGetTypeID(),
        "CFMessagePortGetTypeID", CFMessagePortGetTypeID(),
        "CFNotificationCenterGetTypeID", CFNotificationCenterGetTypeID(),
        "CFNullGetTypeID", CFNullGetTypeID(),
        "CFNumberFormatterGetTypeID", CFNumberFormatterGetTypeID(),
        "CFNumberGetTypeID", CFNumberGetTypeID(),
        "CFPlugInGetTypeID", CFPlugInGetTypeID(),
        "CFPlugInInstanceGetTypeID", CFPlugInInstanceGetTypeID(),
        "CFReadStreamGetTypeID", CFReadStreamGetTypeID(),
        "CFRunLoopGetTypeID", CFRunLoopGetTypeID(),
        "CFRunLoopObserverGetTypeID", CFRunLoopObserverGetTypeID(),
        "CFRunLoopSourceGetTypeID", CFRunLoopSourceGetTypeID(),
        "CFRunLoopTimerGetTypeID", CFRunLoopTimerGetTypeID(),
        "CFSetGetTypeID", CFSetGetTypeID(),
        "CFSocketGetTypeID", CFSocketGetTypeID(),
        "CFStringGetTypeID", CFStringGetTypeID(),
        "CFStringTokenizerGetTypeID", CFStringTokenizerGetTypeID(),
        "CFTimeZoneGetTypeID", CFTimeZoneGetTypeID(),
        "CFTreeGetTypeID", CFTreeGetTypeID(),
        "CFURLEnumeratorGetTypeID", CFURLEnumeratorGetTypeID(),
        "CFURLGetTypeID", CFURLGetTypeID(),
        "CFUUIDGetTypeID", CFUUIDGetTypeID(),
        "CFUserNotificationGetTypeID", CFUserNotificationGetTypeID(),
        "CFWriteStreamGetTypeID", CFWriteStreamGetTypeID(),
        "CFXMLNodeGetTypeID", CFXMLNodeGetTypeID(),
        "CFXMLParserGetTypeID", CFXMLParserGetTypeID(),
        NULL, NULL
    };
#endif /* GUESS_UNKNOWN_TYPES */
    CFTypeID valueType = CFGetTypeID(value);
    
    if (valueType == CFStringGetTypeID()) {
        CFStringAppend(str, value);
    } else if (valueType == CFNumberGetTypeID()) {
        int64_t n;
        CFNumberGetValue(value, kCFNumberSInt64Type, &n);
        CFStringAppendFormat(str, NULL, CFSTR("0x%llx"), n);
    } else if (valueType == CFArrayGetTypeID()) {
        CFStringAppend(str, CFSTR("[ "));
        CFIndex n = CFArrayGetCount(value);
        for (CFIndex k = 0; k < n; k++) {
            if (k > 0) CFStringAppend(str, CFSTR(", "));
            CFTypeRef vv = CFArrayGetValueAtIndex(value, k);
            cfValueToString(str, vv);
        }
        CFStringAppend(str, CFSTR(" ]"));
    } else if (valueType == CFDictionaryGetTypeID()) {
        CFStringAppend(str, CFSTR("{ "));
        CFIndex n = CFDictionaryGetCount(value);
        CFTypeRef *keys = CFAllocatorAllocate(kCFAllocatorDefault, sizeof(CFTypeRef) * n, 0);
        CFTypeRef *values = CFAllocatorAllocate(kCFAllocatorDefault, sizeof(CFTypeRef) * n, 0);
        CFDictionaryGetKeysAndValues(value, keys, values);
        for (CFIndex i = 0; i < n; i++) {
            if (CFGetTypeID(values[i]) == CFBooleanGetTypeID() && !CFBooleanGetValue(values[i]))
                continue;
            
            if (i > 0) CFStringAppend(str, CFSTR(", "));
            cfValueToString(str, keys[i]);
            CFStringAppend(str, CFSTR("="));
            cfValueToString(str, values[i]);
        }
        CFAllocatorDeallocate(kCFAllocatorDefault, keys);
        CFAllocatorDeallocate(kCFAllocatorDefault, values);
        CFStringAppend(str, CFSTR(" }"));
    } else if (valueType == CFBooleanGetTypeID()) {
        CFStringAppend(str, CFBooleanGetValue(value) ? CFSTR("TRUE") : CFSTR("FALSE"));
    } else {
#if GUESS_UNKNOWN_TYPES
        for (struct idMap *l = typeMap;l->name;l++) {
            if (valueType == l->id) {
                CFStringAppendFormat(str, NULL, CFSTR("Type[%s]"), l->name);
                break;
            }
        }
#endif /* GUESS_UNKNOWN_TYPES */
        CFStringAppendFormat(str, NULL, CFSTR("??type=0x%lx?? %@"), valueType, value);
    }
}

void hidElementToString(CFMutableStringRef str, IOHIDElementRef element, char *basePrefix)
{
    CFMutableDictionaryRef props = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFAutorelease(props);
    
    CFStringRef tCFString = IOHIDElementGetName(element);
    
    if (tCFString) {
        CFDictionarySetValue(props, CFSTR("Name"), tCFString);
    }
    
    IOHIDElementType elementType = IOHIDElementGetType(element);
    tCFString = CFSTR("unknown");
    
    switch(elementType) {
        case kIOHIDElementTypeInput_Misc: tCFString = CFSTR("Misc"); break;
        case kIOHIDElementTypeInput_Button: tCFString = CFSTR("Button"); break;
        case kIOHIDElementTypeInput_Axis: tCFString = CFSTR("Axis"); break;
        case kIOHIDElementTypeInput_ScanCodes: tCFString = CFSTR("ScanCodes"); break;
        case kIOHIDElementTypeOutput: tCFString = CFSTR("Output"); break;
        case kIOHIDElementTypeFeature: tCFString = CFSTR("Feature"); break;
        case kIOHIDElementTypeCollection: tCFString = CFSTR("Collection"); break;
    }
    
    CFDictionarySetValue(props, CFSTR("Type"), tCFString);
    
    IOHIDElementCollectionType elementCollectionType = IOHIDElementGetCollectionType(element);
    tCFString = CFSTR("unknown");
    
    switch(elementCollectionType) {
        case kIOHIDElementCollectionTypePhysical: tCFString = CFSTR("Physical"); break;
        case kIOHIDElementCollectionTypeApplication: tCFString = CFSTR("Application"); break;
        case kIOHIDElementCollectionTypeLogical: tCFString = CFSTR("Application"); break;
        case kIOHIDElementCollectionTypeReport: tCFString = CFSTR("Report"); break;
        case kIOHIDElementCollectionTypeNamedArray: tCFString = CFSTR("NamedArray"); break;
        case kIOHIDElementCollectionTypeUsageSwitch: tCFString = CFSTR("UsageSwitch"); break;
        case kIOHIDElementCollectionTypeUsageModifier: tCFString = CFSTR("UsageModifier"); break;
    }
    
    CFDictionarySetValue(props, CFSTR("CollectionType"), tCFString);
    
    uint32_t tInt32 = IOHIDElementGetUsagePage(element);
    tCFString = NULL;
    
    switch(tInt32) {
        case kHIDPage_GenericDesktop: tCFString = CFSTR("GenericDesktop"); break;
        case kHIDPage_Simulation: tCFString = CFSTR("Simulation"); break;
        case kHIDPage_VR: tCFString = CFSTR("VR"); break;
        case kHIDPage_Sport: tCFString = CFSTR("Sport"); break;
        case kHIDPage_Game: tCFString = CFSTR("Game"); break;
        case kHIDPage_GenericDeviceControls: tCFString = CFSTR("DeviceControls"); break;
        case kHIDPage_KeyboardOrKeypad: tCFString = CFSTR("Keyboard"); break;
        case kHIDPage_LEDs: tCFString = CFSTR("LEDS"); break;
        case kHIDPage_Button: tCFString = CFSTR("Button"); break;
        case kHIDPage_Ordinal: tCFString = CFSTR("Ordinal"); break;
        case kHIDPage_Telephony: tCFString = CFSTR("Telephony"); break;
        case kHIDPage_Consumer: tCFString = CFSTR("Consumer"); break;
        case kHIDPage_Digitizer: tCFString = CFSTR("Digitizer"); break;
        case kHIDPage_PID: tCFString = CFSTR("PID"); break;
        case kHIDPage_Unicode: tCFString = CFSTR("Unicode"); break;
        case kHIDPage_AlphanumericDisplay: tCFString = CFSTR("AlphanumericDisplay"); break;
        case kHIDPage_Sensor: tCFString = CFSTR("Sensor"); break;
        case kHIDPage_Monitor: tCFString = CFSTR("Monitor"); break;
        case kHIDPage_MonitorEnumerated: tCFString = CFSTR("MonitorEnumerated"); break;
        case kHIDPage_MonitorVirtual: tCFString = CFSTR("Virtual"); break;
        case kHIDPage_MonitorReserved: tCFString = CFSTR("Reserved"); break;
        case kHIDPage_PowerDevice: tCFString = CFSTR("PowerDevice"); break;
        case kHIDPage_BatterySystem: tCFString = CFSTR("BatterySystem"); break;
        case kHIDPage_PowerReserved: tCFString = CFSTR("PowerReserved"); break;
        case kHIDPage_PowerReserved2: tCFString = CFSTR("PowerReserved2"); break;
        case kHIDPage_BarCodeScanner: tCFString = CFSTR("BarCodeScanner"); break;
        case kHIDPage_Scale: tCFString = CFSTR("Scale"); break;
        case kHIDPage_MagneticStripeReader: tCFString = CFSTR("MangeticStripeReader"); break;
        case kHIDPage_CameraControl: tCFString = CFSTR("CameraControl"); break;
        case kHIDPage_Arcade: tCFString = CFSTR("Arcade"); break;
        case kHIDPage_VendorDefinedStart: tCFString = CFSTR("VendorDefinedStart"); break;
    }
    
    if (!tCFString) {
        tCFString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("<0x%x>"), tInt32);
    }
    
    CFDictionarySetValue(props, CFSTR("UsagePage"), tCFString);
    
    tInt32 = IOHIDElementGetUsage(element);
    CFDictionarySetValue(props, CFSTR("Usage"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    tInt32 = IOHIDElementGetReportSize(element);
    CFDictionarySetValue(props, CFSTR("ReportSize"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    tInt32 = IOHIDElementGetReportCount(element);
    CFDictionarySetValue(props, CFSTR("ReportCount"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    tInt32 = IOHIDElementGetReportID(element);
    CFDictionarySetValue(props, CFSTR("ReportID"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    tInt32 = IOHIDElementGetUnit(element);
    CFDictionarySetValue(props, CFSTR("Unit"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    tInt32 = IOHIDElementGetUnitExponent(element);
    CFDictionarySetValue(props, CFSTR("UnitExponent"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &tInt32));
    
    Boolean tBoolean = IOHIDElementHasNullState(element);
    CFDictionarySetValue(props, CFSTR("HasNullState"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementHasPreferredState(element);
    CFDictionarySetValue(props, CFSTR("HasPreferredState"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementIsArray(element);
    CFDictionarySetValue(props, CFSTR("IsArray"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementIsNonLinear(element);
    CFDictionarySetValue(props, CFSTR("IsNonLinear"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementIsRelative(element);
    CFDictionarySetValue(props, CFSTR("IsRelative"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementIsVirtual(element);
    CFDictionarySetValue(props, CFSTR("IsVirtual"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    tBoolean = IOHIDElementIsWrapping(element);
    CFDictionarySetValue(props, CFSTR("IsWrapping"), (CFBooleanRef)(tBoolean ? kCFBooleanTrue : kCFBooleanFalse));
    
    CFIndex tCFIndexMin = IOHIDElementGetLogicalMin(element);
    CFIndex tCFIndexMax = IOHIDElementGetLogicalMax(element);
    CFDictionarySetValue(props, CFSTR("LogicalRange"), CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%ld-%ld"), tCFIndexMin, tCFIndexMax));
    
    tCFIndexMin = IOHIDElementGetPhysicalMin(element);
    tCFIndexMax = IOHIDElementGetPhysicalMax(element);
    CFDictionarySetValue(props, CFSTR("PhysicalRange"), CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%ld-%ld"), tCFIndexMin, tCFIndexMax));
    
    CFStringAppendCString(str, basePrefix, kCFStringEncodingMacRoman);
    cfValueToString(str, props);
    
    CFArrayRef elementChildren = IOHIDElementGetChildren(element);
    
    if (elementChildren) {
        CFDictionarySetValue(props, CFSTR("HasChildren"), kCFBooleanTrue);
        CFIndex numChildren = CFArrayGetCount(elementChildren);
        CFDictionarySetValue(props, CFSTR("NumChildren"), CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%ld"), numChildren));
        
        for (CFIndex i = 0; i < numChildren; i++) {
            IOHIDElementRef childElement = (IOHIDElementRef)CFArrayGetValueAtIndex(elementChildren, i);
            char *prefix = NULL;
            asprintf(&prefix, "%s[%ld] cookie:0x%04x ", basePrefix, i, IOHIDElementGetCookie(childElement));
            hidElementToString(str, childElement, prefix);
            free(prefix);
            CFStringAppend(str, CFSTR("\n"));
        }
    }
}

void dumpElements(char *basePrefix, IOHIDDeviceRef device, int verbose)
{
    CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
    
    if (!elements) {
        if (verbose) {
            printf("%s No elements found.\n", basePrefix);
        }
        return;
    }
    
    CFAutorelease(elements);
    
    CFIndex numElements = CFArrayGetCount(elements);
    
    printf("%s %ld Elements:\n", basePrefix, numElements);
    
    for (CFIndex i = 0;i < numElements;i++) {
        IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
        
        char *prefix = NULL;
        asprintf(&prefix, "%s cookie:0x%04x ", basePrefix, IOHIDElementGetCookie(element));
        
        CFMutableStringRef buf = CFStringCreateMutable(kCFAllocatorDefault, 1024);
        CFAutorelease(buf);
        hidElementToString(buf, element, prefix);
        printf("%s\n", CFStringGetCStringPtr(buf, kCFStringEncodingMacRoman));
        free(prefix);
    }
}

void dumpDevices(IOHIDDeviceRef *devices, CFIndex numDevices, int verbose)
{
    size_t colWidth = 0;
    const char *properties[] = {
        kIOHIDTransportKey,
        kIOHIDVendorIDKey,
        kIOHIDManufacturerKey,
        kIOHIDVendorIDSourceKey,
        kIOHIDProductIDKey,
        kIOHIDProductKey,
        kIOHIDVersionNumberKey,
        kIOHIDSerialNumberKey,
        kIOHIDCountryCodeKey,
        kIOHIDStandardTypeKey,
        kIOHIDLocationIDKey,
        kIOHIDDeviceUsageKey,
        kIOHIDDeviceUsagePageKey,
        kIOHIDDeviceUsagePairsKey,
        kIOHIDPrimaryUsageKey,
        kIOHIDPrimaryUsagePageKey,
        kIOHIDMaxInputReportSizeKey,
        kIOHIDMaxOutputReportSizeKey,
        kIOHIDMaxFeatureReportSizeKey,
        kIOHIDReportIntervalKey,
        kIOHIDSampleIntervalKey,
        kIOHIDBatchIntervalKey,
        kIOHIDRequestTimeoutKey,
        kIOHIDResetKey,
        kIOHIDKeyboardLanguageKey,
        kIOHIDAltHandlerIdKey,
        kIOHIDBuiltInKey,
        kIOHIDDisplayIntegratedKey,
        kIOHIDProductIDMaskKey,
        kIOHIDProductIDArrayKey,
        kIOHIDPowerOnDelayNSKey,
        kIOHIDCategoryKey,
        kIOHIDMaxResponseLatencyKey,
        kIOHIDUniqueIDKey,
        NULL
    };
    CFStringRef cStr_properties[sizeof(properties)/sizeof(const char *)];
    
    for (int i = 0; properties[i]; i++) {
        cStr_properties[i] = CFStringCreateWithCString(kCFAllocatorDefault, properties[i], kCFStringEncodingASCII);
        CFAutorelease(cStr_properties[i]);
        
        if (strlen(properties[i]) > colWidth) {
            colWidth = strlen(properties[i]);
        }
    }
    colWidth++;
    
    printf("Device Dump:\n\n");
    
    for (CFIndex i = 0; i < numDevices; i++) {
        IOHIDDeviceRef device = devices[i];
        char *prefix = NULL;
        
        asprintf(&prefix, "%02ld ", i);
        
        for (int j = 0;properties[j];j++) {
            const char *valStr = "<notSet>";
            
            CFTypeRef v = IOHIDDeviceGetProperty(device, cStr_properties[j]);
            
            if (!v) {
                if (!verbose) continue;
            } else {
                CFMutableStringRef buf = CFStringCreateMutable(kCFAllocatorDefault, 1024);
                CFAutorelease(buf);
                cfValueToString(buf, v);
                valStr = CFStringGetCStringPtr(buf, kCFStringEncodingMacRoman);
            }
        
            printf("%s %-*s: %s\n", prefix, (int)colWidth, properties[j], valStr);
        }
        dumpElements(prefix, device, verbose);
        printf("\n");
    }
}

int main(int argc, char * argv[]) {
    uint8_t wasdColor = 0, colorRed = 0, colorGreen = 0, colorBlue = 0;
    int opt_ch;
    int option_dump = 0;
    int option_verbose = 0;
    
    static struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "dump", no_argument, NULL, 'd' },
        { "verbose", no_argument, NULL, 'v' },
        { "color", required_argument, NULL, 'c' },
        { "rgb", required_argument, NULL, 'C' },
        { NULL, 0, NULL, 0 }
    };
    
    if (argc == 1)
        usage(1, argv, longopts);
    
    while ((opt_ch = getopt_long(argc, argv, "hdvc:C:", longopts, NULL)) != -1) {
        switch (opt_ch) {
            case 'd':
                option_dump = 1;
                break;
            case 'v':
                option_verbose++;
                break;
            case 'h':
                usage(0, argv, longopts);
                break;
            case 'c':
                colorRed = colorGreen = colorBlue = atoi(optarg);
                break;
            case 'C':
                parseColor(optarg, &colorRed, &colorGreen, &colorBlue);
                break;
            default:
                usage(1, argv, longopts);
                break;
        }
    }
    
    argc += optind;
    argv += optind;
    
    if (option_verbose >= 4)
        option_dump = 1;
    
    IOHIDManagerRef hidManagerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    
    if (!hidManagerRef) {
        fprintf(stderr, "Failed to get IOHIDManager!\n");
        exit(2);
    }
    
    CFAutorelease(hidManagerRef);
    
    // Setup matching to get correct device(s)
    CFMutableDictionaryRef matchDictRef = setMatchSelection(NULL, CFSTR(kIOHIDDeviceUsagePageKey), kHIDPage_GenericDesktop);
    
    setMatchSelection(matchDictRef, CFSTR(kIOHIDDeviceUsageKey), kHIDUsage_GD_Keyboard);
    
    if (!option_dump) {
        // Look for Logitech (0x046d) with Vendor extensions (DeviceUsagePage=0xff00, DeviceUsage=0x0)
        setMatchSelection(matchDictRef, CFSTR(kIOHIDVendorIDKey), 0x046d);
        
        CFMutableDictionaryRef deviceUsagePairs = CFDictionaryCreateMutable(kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        int32_t matchUsagePage = 0xff00, matchUsage = 0x0;
        CFDictionarySetValue(deviceUsagePairs, CFSTR("DeviceUsagePage"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &matchUsagePage));
        CFDictionarySetValue(deviceUsagePairs, CFSTR("DeviceUsage"), CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &matchUsage));
        CFMutableArrayRef deviceUsagePairsArray = CFArrayCreateMutable(kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks);
        CFArrayAppendValue(deviceUsagePairsArray, deviceUsagePairs);
        CFDictionarySetValue(matchDictRef, CFSTR("DeviceUsagePairs"), deviceUsagePairsArray);
    }
    
    IOHIDManagerSetDeviceMatching(hidManagerRef, matchDictRef);
    CFRelease(matchDictRef);
    matchDictRef = NULL;
    
    // Open the device manager
    IOReturn retVal = IOHIDManagerOpen(hidManagerRef, kIOHIDOptionsTypeNone);
    
    if (retVal) {
        fprintf(stderr, "Failed to open HID device manager!\n");
        exit(4);
    }
    
    CFSetRef devicesFound = IOHIDManagerCopyDevices(hidManagerRef);
    
    if (!devicesFound) {
        fprintf(stderr, "Failed to copy out devices that were found!\n");
        exit(5);
    }
    
    CFIndex numDevices = CFSetGetCount(devicesFound);
    
    IOHIDDeviceRef *devices = malloc(sizeof(IOHIDDeviceRef) * numDevices);
    
    if (!devices) {
        fprintf(stderr, "Failed to allocate memory for device list!\n");
        CFRelease(devicesFound);
        exit(6);
    }
    
    CFSetGetValues(devicesFound, (const void **)devices);
    CFRelease(devicesFound);
    devicesFound = NULL;
    
    int exitValue = 0;
    
    if (option_dump)
        dumpDevices(devices, numDevices, option_verbose > 4 ? 1 : 0);
    else
        for (CFIndex i = 0;i < numDevices;i++) {
            CFNumberRef productIdRef = IOHIDDeviceGetProperty(devices[i], CFSTR(kIOHIDProductIDKey));
            int32_t productId = 0, cookieId = 0;
            CFNumberGetValue(productIdRef, kCFNumberSInt32Type, &productId);
            
            if (productId == 0xc24d) {
                // Logitech G710+ not sure about others yet
                cookieId = 0x11d;
            }
            
            if (cookieId == 0) {
                printf("Skipping unknown productId = 0x%x\n", productId);
                continue;
            }
            
            matchDictRef = setMatchSelection(NULL, CFSTR(kIOHIDElementCookieKey), cookieId);
            CFAutorelease(matchDictRef);
            
            CFArrayRef elements = IOHIDDeviceCopyMatchingElements(devices[i], matchDictRef, kIOHIDOptionsTypeNone);
            
            if (!elements) {
                if (option_verbose) {
                    fprintf(stderr, "NOTE: Device[%lx] did not have any matching elements\n", (unsigned long)devices[i]);
                }
                continue;
            }
            
            CFAutorelease(elements);
            
            CFIndex numElements = CFArrayGetCount(elements);
            
            for (CFIndex j = 0;j < numElements;j++) {
                IOHIDElementRef elementRef = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, j);
                
                if (!elementRef) {
                    continue;
                }
                
                uint64_t timestamp = 0;
                uint8_t usb_data[] = { wasdColor, colorRed, colorGreen, colorBlue };
                
                IOHIDValueRef valueRef = IOHIDValueCreateWithBytes(kCFAllocatorDefault, elementRef, timestamp, usb_data, sizeof(usb_data));
                
                if (valueRef) {
                    retVal = IOHIDDeviceSetValue(devices[i], elementRef, valueRef);
                    CFRelease(valueRef);
                    
                    if (retVal) {
                        fprintf(stderr, "WARNING: DeviceSetValue returned %d\n", retVal);
                        exitValue = 100;
                    }
                }
            }
        }
    
    free(devices);
    exit(exitValue);
}