# m4gpiler ( A marthon (or C, idk) compiler to URCL )
For how it's just an example calculator there is still a lot of work to do. Further development will be done in the `compilerattempt` branch until 
something decent comes out of it

## Compiling
The following tool is required to compile: https://github.com/m4gh3/cgnale , and because of that for now building can't be done outside of Linux/WSL (only Linux tested)

Also the installation of such tool is quite tricky (however the source code of such tool is below 600 lines I believe)
(The irony of making somenthing opensource, but you still care manly about your own comfort (?) )

Any feedback/pull request both to this project and cgnale is appreciated

You will also need g++ and make and it should be it, just type `make`

## How does the current example work?

After you built the current m4gpiler run the command `./m4gpiler` in this repo directory.

You can type expressions like `(1+2)*3+1+2` and it should display the result.

Be warned: only the operators `*` and `+` are supported,

after all as I said that's just a demo.
