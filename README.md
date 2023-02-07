# LegionCore

#### Table of Contents
* [Introduction](#introduction)
* [Requirements](#requirements)
* [Install](#install)
* [Data Files](#data-files)
* [Common Issues](#common-issues)
* [Reporting issues](#reporting-issues)
* [Submitting fixes](#submitting-fixes)
* [Thank you](#thank-you)

#### Introduction
LegionCore is a **MMORPG** framework for WOW Legion *(Build 26972)*. This core is based off of the UWOW core leak of 2020. Which was derived from an old version of [TrinityCore](https://github.com/TrinityCore/TrinityCore). LegionCore is completely opensource and is developed by the community. To submit a pull request please follow this template [here](submitting-fixes).

If you want you can join the community discord: [here](https://discord.gg/uaP2aeJ7sj).

# Requirements
 
 ### Important
 The main difference to note is that LegionCore requires Boost (1.64.0), Visual Studio 2017 or later, and MySQL (5.6.51).
 
[Windows specific](https://www.trinitycore.info/en/install/requirements/windows)
  
[Linux specific](https://www.trinitycore.info/en/install/requirements/linux)

[Mac specific](https://www.trinitycore.info/en/install/requirements/macos)

# Install
Most of the install steps are the same as the TrinityCore ones [here](https://www.trinitycore.info/en/install/Core-Installation).

# Data Files
This core has been optimized for DBC/vmap/map/mmaps files from UWoW and are not provided as part of this source code package. Instead the data files for LegionCore can be acquired [here](https://www.emucoach.com/legion-7-3-5-/6945-repack-7-3-5-legion-wow-repack-wow-legion-7-3-5-repack-blizzlike-fun.html).

***⚠️ We will not provide further assistance if those files are not available anymore.***

Some files from the above data files are wrong.
Use the [TrinityCore 7.3.5 tools](https://github.com/TrinityCore/TrinityCore/releases/tag/7.3.5%2F26972) to generate the following files:
> gt/xp.txt

Replace the listed above file(s) with the one(s) you generated yourself.

# Common issues
TODO

# Reporting issues
Issues can be reported via the [Github issue tracker](https://github.com/dufernst/LegionCore-7.3.5/issues).

Please take the time to review existing issues before submitting your own to
prevent duplicates.

In addition, thoroughly read through the [issue tracker guide](https://community.trinitycore.org/topic/37-the-trinitycore-issuetracker-and-you/) to ensure
your report contains the required information. Incorrect or poorly formed
reports are wasteful and are subject to deletion.

Note that the issue tracker guide is from TrinityCore, but it also applies for this core.

# Submitting fixes
C++ fixes are submitted as pull requests via Github. For more information on how to
properly submit a pull request, read the [how-to: maintain a remote fork](https://community.trinitycore.org/topic/9002-howto-maintain-a-remote-fork-for-pull-requests-tortoisegit/).
For SQL only fixes, open a ticket; if a bug report exists for the bug, post on an existing ticket.

### Thank you
- [TrinityCore Authors](https://github.com/TrinityCore/TrinityCore/blob/master/AUTHORS)
- [LegionCore Contributors](https://github.com/dufernst/LegionCore-7.3.5/graphs/contributors)

> **License: GPL 2.0** read [COPYING](COPYING).
