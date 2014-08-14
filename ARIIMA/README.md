ARIIMA
======

This repository contains the ARIIMA architecture used to generate and process the weekly usage forecast for Eco-aware Smart Everyday Things.
Different objects are able to ask them weekly energy working mode through an iBoard Pro (a more complete Arduino Board with Ethernet shield and RTC clock embedded).
A RESTful server processes the request (GET) and send the prediction to the object-client.

##Installation

To execute the forecast operation, it is necessary to download and install [R-project](http://www.r-project.org).

To execute the server, it is necessary to download and install [Flask](http://flask.pocoo.org) and [Requests](http://docs.python-requests.org/en/latest/) using these commands:
	
	pip install Flask
	pip install requests

You have to confing your Flask configuration.

##Directories

* generateForecast contains the files used to automate the time series forecast execution (every sunday).
 * generateForecast.py is the python script in which the last 23 working days are retrieved and used to generate the weekly usage for the next 5 working days. The prediction is stored in CouchDB database.
 * timeseries_coffees.r is the R script used by generateForecast.py file.
* getPrediction_and_switch is Arduino code. It does these operations:
 1. make an HTTP GET request (i.e. `http://dev.morelab.deusto.es/ecoserver/predictions/{deviceID}`) to obtain the weekly usage;
 2. store the prediction in EEPROM memory;
 3. apply the prediction every slot time for each day.
* serverSide contains the server code.
 * miniserver.py is the server.
