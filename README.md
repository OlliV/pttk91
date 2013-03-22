PTTK91 VM
=========

PTTK91 is a portable implementation of TTK91 specification written in C. One of
PTTK91's goals is to integrate it with
<a href="https://github.com/Ninjaware/zeke">zeke</a> project as an protected
runtime environment for it. This is interesting because zeke it self can't
offer any memory protection or privilege levels due to the limits of Cortex-M0.


Configure and Build
-------------------

Build configuration is stored in a file called config.

After configuration is modified, especially correct target is selected,
the vm is built with make.
