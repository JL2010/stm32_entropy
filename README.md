stm32_entropy
=============

A clock-jitter based random number generator using an STM32F100C8 microcontroller and its internal clock sources.

With the RTC being clocked off of the low-speed internal oscillator (LSI), the RTC pre-scaler overflow interrupt was used to periodically sample the SysClk counter that was driven off of the HCLK (derived from separate high-speed internal oscillator (HSI)).

Samples were run through Ilja Gerhardt's NIST testsuite in Python with good results: http://gerhardt.ch/random.php

While this can potentially be used as a building block for protocols which require a cryptographic nonce, do not use this for secure applications without conducting your own indpendant testing.

Dependancies:
=============
  CodeSourcery ARM tool chain
  STM32 Standard Peripheral Libarary

Initialization code was largely based on Pandafruits excellent STM32 primer: http://pandafruits.com/stm32_primer/stm32_primer_minimal.php