ARIIMA
======

This repository contains the ARIIMA architecture used to generate and process the weekly usage forecast for Eco-aware Smart Everyday Things.

##Installation

To execute the forecast operation, it is necessary to download and install [R-project](http://www.r-project.org).

##Directories

* generateForecast contains the files used to automate the forecast execution (every sunday).
 * generateForecast.py is the python script in which the last 23 working days are retrieved and used to generate the weekly usage for the next 5 working days. The prediction is stored in CouchDB database.
 * timeseries_coffees.r is the R script used by generateForecast.py file.
* getPrediction_and_switch is Arduino code to execute these operations: 1) make an HTTP GET request (i.e. `http://dev.morelab.deusto.es/ecoserver/predictions/{deviceID}`) to obtain the weekly usage; 2) store the prediction in EEPROFM memory; 3) apply the prediction every slot time for each day.