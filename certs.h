#ifndef certs_h
#define certs_h

// Amazon's root CA. This should be the same for everyone.
const char AWS_CERT_CA[] = "-----BEGIN CERTIFICATE-----\n" \
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
"rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
"-----END CERTIFICATE-----\n";

// The private key for your device
const char AWS_CERT_PRIVATE[] = "-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpAIBAAKCAQEArhOsJt2sCMpC+SRiXOBmhI+UVCO9QWIn4RK5NPimiVeDbAJC\n" \
"j8FjP+IPthpavpixWLYWP4FOTvY5zHffr/W/lnsslnaDWAn0lgPNPf6fBixkOcDN\n" \
"g1zsSCoFwRF2X7ePC43CYou65uoqw04Cd727V86P4fObN36M4cxsYZbuVAXP6iRn\n" \
"50QeuME4k3gN7yDqJG9x8tBDqTEu7uZgM9bARJEe1tRn/pUtQ+qkZhppj+DA4z5G\n" \
"2UcByqSqNobOfKSAg36V+tENIDBJjkNfS5CPTUAu04MNRVpngj8ogJ1j5l1D0Ml8\n" \
"ADNQ+02KbnEpfanrjxJ3Won8LiFHgWA+PHY9xQIDAQABAoIBAHy5A6NlOBmkMZAo\n" \
"jYFWFp3r8a7HXJY4KbKNo71cWpYU6ci9ZctyGBZxdihXgaUF6itRa2dc0iqpKIhj\n" \
"axW70eWTo0K67yhpGaFSX/+HK5FUxQrMJXN1ZKXf1fea92FlN0xj6V6KFwPTy4fb\n" \
"oNECU/R5v4V48UE1nuM5gH37kg8IXEDlJUxTjk/XIl3YudhOLvaFBvqjMFJUqy40\n" \
"C55E/AYrOuhU8Zeb49BcXUfasGr8Brt1YjX2nETP30ImAuTVqoCDI9z4s+77+N6M\n" \
"ktnrWysV4Z44W/UDStB7QoGSlsiiyAMagdjjOeDZrMs3GX7FFhO09P+I1hTp513d\n" \
"mAh3WxECgYEA3IPeXDeS1lBV+7zw0mtsX0UnqZ+83bdpao4uZ+Oa+yPOe6KlpWgb\n" \
"x+uYNBiCGa2pZM0+PnM0wNFsKHsdBIlC1sPBtBq7pbaqV1segYhxAcFYzsAf246O\n" \
"cu/YozG4zmgJLSa2T4Psp75UNpQCDUGrON1RewNufdO1dHl4qxk9/LsCgYEAyhbG\n" \
"8E9Gt15V/Cayt/cRS11Ojnh8Ysu+DfDv1MxFqH/HwaZTH/F9wrPCsg1OYfP8sBHr\n" \
"5S7T+xVJUfDFBsG48N+nv6iUfnbveV8k287HP3CjdmJoWcn91JTPT9/5JYmHcSfk\n" \
"LFB1Zs/JFH9HVEqoSp+5ZI5sZCGA2Gsq8zX6R38CgYEAuVqCmsL/wvMOAybiygwP\n" \
"sNjhKNwjFb6odi+C5EJVsAl980BYufsTdu0aJ5f+K3VksF0wl6+gxP6DSleJoZl6\n" \
"vCkZdbkOx5EOB5WIw9qbGp2Zd1fYr2VZaVUfpLUToTOjmvcyQQ4j6y+5e5LXUqtD\n" \
"L+4gIiVVR6CfooLEIhV6nCcCgYBYPJ+6N3MSB1+VzDPGfVsQw/nT3Sm6XWlrXuLo\n" \
"0BQuITTQCdZLoT6e76w33/V0cXrISdrs44htHfs4it8MNzOQAKeW2H2VxkqDfRZz\n" \
"3xJCUeUdgJQ9eCObWSo3nvyC9kysDJXk/3BzZpBOodA8LqjShHEHReRvx+0hEZHc\n" \
"RZo08wKBgQDA8CkZePaflvzhj1mmk6O+kRhb2XDst4qgkzbXFTvHCtbOqMCWj+QA\n" \
"iYOrq+biyI/noIHCZHtvP+pSLvGzdEoJbJEdtTiJlr/ON84Rn/oxd9a8QtBIugqb\n" \
"uurnZf4R7530+jY5wII7++kXfLFmjO2cW4USHXcR9/lMTrpXlq4T1A==\n" \
"-----END RSA PRIVATE KEY-----\n";

// The certificate for your device
const char AWS_CERT_CRT[] = "-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAIEjFJtTcTuh9ZxVHz9+lBxuMDzQMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yMTAxMTQxNDA4\n" \
"MjVaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDWGef7veuV2ZABTNoo\n" \
"ILJY2utJYlhIMo0fdnPL6NS2XYrGdRntbb00onhia/DusbxJ5UEolU/z18WsBrx+\n" \
"G5W9EC+HslSL14OkB1HzeIPVlJIqMCfylCLk7aWA/TCWoEPKBO5uV3IoiNnixbNm\n" \
"q16ByGbA9YFaQ00qwOBK10w4vhGVssSvuRczRbKZFWnLKh8LunawDg3JxxvWStQZ\n" \
"kRy9/J40jdcJaABP0UkFYSfdxH34+ZuWqhGSvXr9hR7Grws1+T0/X/AjD98SvBKn\n" \
"YhOv8jv53t6XEY+3rJB5D+Vgoc4kNAHFhX3WkXV6Hdoq4pzKjMDvfqpYcKfKoOEa\n" \
"o4pDAgMBAAGjYDBeMB8GA1UdIwQYMBaAFBNqjrNwrM3P9p/uK8C1koRiWK96MB0G\n" \
"A1UdDgQWBBTHCIxMvfkb2kvjZvah092Zw9IbFzAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEADFfYRoyj1DDQpUlu1azupecX\n" \
"44gP9GMK1/vLkcDP6r9PcEbKJEWF9LfX6gc1jN3hlRI4asP5E7ghGAvzUimGpF5V\n" \
"84Ue6XI+XDI5MHovBi3PC1eMidEJzQUmI1MB9OKLB0GTdo8E8tezIkdYbdViX1TI\n" \
"UDD7/sUnE5e1pXF2zs9xI4l5eqxjlEwtc7e+VkxoUnQBzcIqyKopdfIxwYQ+Garp\n" \
"KYPQvBNjOIgnJkgyWUtebxGjGuXRz6dnxuP3jEejqkZ8ScVoFe/RE1QGo3AM1ApX\n" \
"IAhUOS0i9/86Dohi6YB350kOM/gPF/9L0ldhuv2W4d0ac/8Dq2EMtEjnWBS4VA==\n" \
"-----END CERTIFICATE-----";

#endif