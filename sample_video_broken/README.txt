This version works only with libavcodec53

'avcodec_decode_audio3’ is deprecated, need review !

diff /tmp/good /tmp/bad 
2c2
< Complete name                            : chasehq_good.mp4
---
> Complete name                            : chasehq_bad.mp4
4,6c4,6
< Format profile                           : Base Media
< Codec ID                                 : isom
< File size                                : 8.61 MiB
---
> Format profile                           : Base Media / Version 2
> Codec ID                                 : mp42
> File size                                : 8.60 MiB
9c9
< Overall bit rate                         : 2 295 Kbps
---
> Overall bit rate                         : 2 293 Kbps
12d11
< Writing application                      : Lavf54.20.4
15c14
< ID                                       : 1
---
> ID                                       : 2
43c42
< ID                                       : 2
---
> ID                                       : 1
48,49c47,48
< Duration                                 : 31s 446ms
< Bit rate mode                            : Variable
---
> Duration                                 : 31s 445ms
> Bit rate mode                            : Constant

