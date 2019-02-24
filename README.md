```shell
git clone https://github.com/ammarfaizi2/esteh
cd esteh
make
sudo cp esteh /usr/bin/esteh && sudo chmod +x /usr/bin/esteh

# Hide debug message with "2>/dev/null"
esteh test.tea 2>/dev/null

# Debug message will be shown.
esteh test.tea

#
# The debug message can be turned off through include/esteh_debug.h
#
```