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

#include <stdio.h>
#include <getopt.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDLib.h>

#define VERBOSE 0

static CFMutableDictionaryRef setMatchSelection(CFMutableDictionaryRef dictRef, CFStringRef key, UInt32 value)
{
    if (dictRef == NULL) {
        dictRef = CFDictionaryCreateMutable( kCFAllocatorDefault,
                                            0,
                                            &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks );
        CFAutorelease(dictRef);
    }
    
    if (dictRef) {
        CFNumberRef tCFNumberRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &value);
        CFDictionarySetValue(dictRef, key, tCFNumberRef);
        CFRelease(tCFNumberRef);
    }
    
    return dictRef;
}

int main(int argc, char * argv[]) {
#pragma unused ( argc, argv )
    uint8_t colorRed = 0, colorGreen = 0, colorBlue = 0;
    int opt_ch;

    static struct option longopts[] = {
        { "version", no_argument, NULL, 'v' },
        { "help", no_argument, NULL, 'h' },
        { "color", required_argument, NULL, 'c' },
        { "rgb", required_argument, NULL, 'C' },
        { NULL, 0, NULL, 0 }
    };
    
    while ((opt_ch = getopt_long(argc, argv, "vhc:C:", longopts, NULL)) != -1) {
        switch (opt_ch) {
        case 'v':
            printf("VERSION: 0.0.1\n");
            exit(0);
            break;
        case 'h':
            printf("Usage: %s [--version][--help][--color {0-255}][--rgb {0-255},{0-255},{0-255}]\n", argv[0]);
            exit(0);
            break;
        case 'c':
            colorRed = colorGreen = colorBlue = atoi(optarg);
            break;
        }
    }
    
    argc += optind;
    argv += optind;
    
    IOHIDManagerRef hidManagerRef = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    
    if (!hidManagerRef) {
        fprintf(stderr, "Failed to get IOHIDManager!\n");
        exit(2);
    }
    
    CFAutorelease(hidManagerRef);
    
    // Setup matching to get correct device(s)
    CFMutableDictionaryRef matchDictRef = setMatchSelection(NULL, CFSTR(kIOHIDDeviceUsagePageKey), kHIDPage_GenericDesktop);
    
    if (!matchDictRef) {
        fprintf(stderr, "Failed to alloc matching dictionary!\n");
        exit(3);
    }
    
    setMatchSelection(matchDictRef, CFSTR(kIOHIDDeviceUsageKey), kHIDUsage_GD_Keyboard);
    setMatchSelection(matchDictRef, CFSTR(kIOHIDVendorIDKey), 0x046d);
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
    
    // Logitech G710+ not sure about others yet
    matchDictRef = setMatchSelection(NULL, CFSTR(kIOHIDElementCookieKey), 0x11d);
    CFAutorelease(matchDictRef);
    
    for (CFIndex i = 0;i < numDevices;i++) {
        CFArrayRef elements = IOHIDDeviceCopyMatchingElements(devices[i], matchDictRef, kIOHIDOptionsTypeNone);
        
        if (!elements) {
#if VERBOSE
            fprintf(stderr, "NOTE: Device[%lx] did not have any matching elements\n", (unsigned long)devices[i]);
#endif /* VERBOSE */
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
            uint8_t usb_data[] = { 5, colorRed, colorGreen, colorBlue };
            
            IOHIDValueRef valueRef = IOHIDValueCreateWithBytes(kCFAllocatorDefault, elementRef, timestamp, usb_data, sizeof(usb_data));
            
            if (valueRef) {
                retVal = IOHIDDeviceSetValue(devices[i], elementRef, valueRef);
                CFRelease(valueRef);
                
                if (retVal) {
                    fprintf(stderr, "WARNING: DeviceSetValue returned %d\n", retVal);
                }
            }
        }
    }
    
    free(devices);
    exit(0);
}