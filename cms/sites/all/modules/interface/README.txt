--- SUMMARY ---

Interface is a tool for designing custom layouts for entities. It is built with the 
non-programmer in mind, and includes drag and drop plug ins for rapidly modelling
the look and feel of various forms of information.

For a full description of what is and is not supported, please see the project page:
  http://www.drupal.org/project/interface

--- REQUIREMENTS ---

* jquery_ui

--- INSTALLATION ---

* Install the module in your modules directory, typically 
  sites/all/modules.

* Enable the module at Administer > Site Building > Modules.

* Select the entities where interface will be active on the Interface activation page
  at Administer > Configuration > User Interface > Interface.

* Edit an enabled entity and select the Manage Interface link. This will appear next
  to links for Manage Display and Manage Fields for most entity types.

* On the Manage Interface page, select a plug-in and create an interface. Plug ins
  provide different types of layouts.

--- PREVIOUS VERSIONS ---

There is no way to upgrade Interface from previous versions. This is because 
interface is designed to work with entities instead of forms, which were the subject 
of previous versions.

--- PLUG-INS ---

Interface is designed to provide a uniform method for supporting alternative layouts
in Drupal. By default, Drupal lays out content in a 'stacked' interface, where 
fields of information are stacked one on top of another. Interface changes this by 
allowing administrators to use alternate displays of information that dramatically 
depart from this style of presentation.

Interface does this through the use of plugins. Plug-ins are defined by hooks in the
interface module, allowing them to be installed just about anywhere in your system. 
The primary hook for creating a plug in is hook_interface_component, which passes
back a keyed array containing information and callbacks about the interface.

Data for plug ins is stored in a simple array in the database. There is nothing to stop
a plug in author from storing information in a custom location, but the data column
of the interface table must be populated at some point in order for the module to
work correctly.

See plugins/interface_tabs for examples.

--- SIMILAR PROJECTS ---

Other Drupal projects offer similar functionality and are very good at what they do.
Interface has been designed primarily to support use cases where people need to rapidly
model the presentation of non-content entities and lack a background managing 
interfaces in Drupal.

If you are considering using interface, you may want to look at these as well:

* Display Suite: http://drupal.org/project/ds
* Design Kit: http://drupal.org/project/designkit

--- CONTACT ---

Current maintainer:

Michael Haggerty (techsoldaten)

Contributors welcome.

