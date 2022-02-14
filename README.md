* [Introduction](#introduction)
* [Requirements](#requirements)
* [Install](#install)
* [Data Files](#data-files)
* [Reporting issues](#reporting-issues)
* [Submitting fixes](#submitting-fixes)
* [Copyright](#copyright)

## Introduction

LegionCore is an *MMORPG* Framework for Legion (26972) based on the uwow.biz core leaked early 2020.
LegionCore itself has been derived from an old version of TrinityCore.

TrinityCore is a *MMORPG* Framework based mostly in C++.

TrinityCore is derived from *MaNGOS*, the *Massive Network Game Object Server*, and is
based on the code of that project with extensive changes over time to optimize,
improve and cleanup the codebase at the same time as improving the in-game
mechanics and functionality.

LegionCore development is completely open source; community involvement is highly encouraged.


## Requirements

The software requirements for LegionCore are very similar to the one from TrinityCore.
The main difference worth mentioning is that on Windows currently BOOST version 1.64 is required.

Software requirements are available in the [wiki](https://www.trinitycore.info/display/tc/Requirements) for
Windows, Linux and macOS.


## Install

Detailed installation guides are available in the [wiki](https://www.trinitycore.info/display/tc/Installation+Guide) for
Windows, Linux and macOS.


## Data Files

This core has been optimized for DBC/vmap/map/mmaps files from UWoW and are not provided as part of this source code package.
More information on how to acquire those files can be found [here](https://www.emucoach.com/legion-7-3-5-/6945-repack-7-3-5-legion-wow-repack-wow-legion-7-3-5-repack-blizzlike-fun.html)
We will not provide further assistance if those files are not available anymore.

At least certain files from the linked data files are wrong.
Use the TrinityCore 7.3.5 tools, of which the source code can be found [here](https://github.com/TrinityCore/TrinityCore/releases/tag/7.3.5%2F26972) to generate the following files:
* gt/xp.txt

Replace the listed above files with the ones you generated yourself.

## Reporting issues

Issues can be reported via the [Github issue tracker](https://github.com/dufernst/LegionCore-7.3.5/issues).

Please take the time to review existing issues before submitting your own to
prevent duplicates.

In addition, thoroughly read through the [issue tracker guide](https://community.trinitycore.org/topic/37-the-trinitycore-issuetracker-and-you/) to ensure
your report contains the required information. Incorrect or poorly formed
reports are wasteful and are subject to deletion.

Note that the issue tracker guide is from TrinityCore, but it also applies for this core.


## Submitting fixes

C++ fixes are submitted as pull requests via Github. For more information on how to
properly submit a pull request, read the [how-to: maintain a remote fork](https://community.trinitycore.org/topic/9002-howto-maintain-a-remote-fork-for-pull-requests-tortoisegit/).
For SQL only fixes, open a ticket; if a bug report exists for the bug, post on an existing ticket.


## Copyright

License: GPL 2.0

Read file [COPYING](COPYING).
