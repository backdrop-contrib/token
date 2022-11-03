Token
=====

**Important note**: Token module was moved into Backdrop core in version 1.1.0. 
Use of this module is no longer necessary and no further changes will be made 
here. If you would like to report a bug or feature request against Token module, 
file an issue in the main Backdrop CMS core repository at https://github.com/backdrop/backdrop-issues.

Backdrop includes the ability to replace small snippets of text with
dynamic replacements. These replaceable snippets are called "tokens", and
can usually be identified by a word surrounded by square brackets, such as
`[node:nid]`. Although core provides the token replacement mechanism, it does
not include a UI for browsing tokens, nor tokens for a lot of its own core
modules. This project provides the missing tokens for core, and adds a UI.

Installation
------------

- Install this module using the official Backdrop CMS instructions at
  https://backdropcms.org/guide/modules

- This module has no settings page, though it does modify a number of forms
  throughout core to include a link to the token browser.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Current Maintainers
-------------------

- Eduardo Garcia (https://github.com/enzolutions/)
- Nate Haug (https://github.com/quicksketch/)

Credits
-------

Ported by Backdrop by Eduardo Garcia.

This module was originally written and maintained for Drupal by:

- Dave Reid (https://www.drupal.org/u/dave-reid)
- Greg Knaddison (https://www.drupal.org/u/greggles)
- Jeff Eaton (https://www.drupal.org/u/eaton)
