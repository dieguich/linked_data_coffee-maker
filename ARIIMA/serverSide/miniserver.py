#!/usr/bin/env python

from flask import Flask, request

import SimpleHTTPServer
import SocketServer
import sys, string, cStringIO, cgi, time, datetime
import logging, couchdb
import ConfigParser, json, time
from   urlparse import urlparse, parse_qs
import traceback

import rdflib
import requests
from requests.auth import HTTPBasicAuth
from datetime import datetime, timedelta

CONSUMPTION_TYPES = {
	'START_TIME': 'sc:Start',
	'STANDBY': 'sc:StandBy',
	'COFFEE': 'sc:Coffee'
}

GRAPH_URI = 'http://helheim.deusto.es/linkedcoffeemachine/resource/'
MACHINE_BASE_URI = GRAPH_URI + 'machine'
PERSON_BASE_URI = GRAPH_URI + 'person'
SPARUL_URI = 'http://helheim.deusto.es/DAV/home/coffeemachine/prueba_post'
USERNAME = 'coffeemachine'
PASSWD = 'c0ff33'

def get_json():
	if request.json is not None:
		return request.json
	else:
		try:
			if request.data:
				data = request.data
			else:
				keys = request.form.keys() or ['']
				data = keys[0]
			return json.loads(data)
		except:
			logger.error("Invalid JSON found")
			print u"Suggested JSON: %(rdata)r" % dict(rdata=data)
			traceback.print_exc()
			return None

app = Flask(__name__)

@app.route('/ecoserver/', methods = ['GET','POST'])
def index():
	if request.method == 'GET':
		return open('index.html').read()

	logger.warning("\n\t***********\n" + datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
	logger.warning("======= POST STARTED =======")
#   logger.warning(request.headers) TODO

	try:
		toUpdate = get_json()
		print toUpdate
		#print toUpdate["time_secs"]
		#print type(toUpdate["time_secs"])

		'''try:
			#toUpdate["time_hours"] = time.strftime('%H:%M:%S', time.gmtime(int(toUpdate["time_secs"])))
			#logger.warning("\n\n"+toUpdate["time_hours"])
		except:
			print "does not work the conversion"
			#toUpdate["time_hours"] = toUpdate["time_secs"]'''

		uploadCouchDB(toUpdate)

		#print "jon code"
		print json_to_rdf(toUpdate)
		logger.info("All OK. Data Base updated and RDF created")

		return "Ok"
	except:
		print "Error in POST"
		logger.error("Error in POST\n")
		logger.error(request.data)
		return "Error in POST"

@app.route('/ecoserver/predictions/<deviceID>', methods = ['GET'])
def prediction(deviceID):
	return getPredictionFromDB(deviceID)

#@app.route('/math/<int:num1>/<int:num2>/')
#def math(num1, num2):
#    return str(num1 / num2)

''' HTTP '''
def httpd():
	ip   = configFile.get("myconfig", "srv_ip")
	port = configFile.getint("myconfig", "srv_port")

	app.run(host = ip, port = port)
   
	print "Server started on", ip, ":", port
	#db.save(jsonTocaPelotas)

''' NoSQL '''
def uploadCouchDB(jsonToUpload):
  try:
	db    = couch[configFile.get("myconfig", "dbname")] # existing database
	db.save(jsonToUpload)
	couch.replicate('eco-aware_devices', 'eco-aware_devices-replicate', continuous=True, filter= "myddoc/myfilter")
	print "saved_in_database"
  except err:
	print "error in cocuhdb: "
	print str(err)

def getPredictionFromDB(deviceID):
    try:
		db         = couch[configFile.get("myconfig", "predictionDB")] # existing database
		prediction = [db[docs]['prediction'] for docs in db if "deviceID" in db[docs] and db[docs]["deviceID"] == deviceID]
    except:
    	logger.error("Error in GET prediction")
    	print traceback.print_exc()
    	print sys.exc_info()[0]
    return str(prediction[0])

''' RDF '''
def insert_by_post(rdf, graph_name, endpoint_url, username, passwd):
	g = rdflib.Graph()
	g.parse(data=rdf, format='turtle')

	triples = ''
	for s, p, o in g.triples((None, None, None)):
		triple = "%s %s %s . " % (s.n3(), p.n3(), o.n3())
		triples += triple

	query = 'INSERT IN GRAPH <%s> { %s }' % (graph_name.encode('utf-8'), triples.encode('utf-8'))

	r = requests.post(endpoint_url, data=query, headers={'Content-type': 'application/sparql-query'},
		auth=HTTPBasicAuth(username, passwd))

	if r.status_code != 201:
		raise Exception("Update failed")

def json_to_rdf(jsondict):
	try:
		print "previous to insert 1"
		jsondict['machine_base_uri'] = MACHINE_BASE_URI

		start_date = datetime.strptime(jsondict['date'] + 'T' + jsondict['time'], '%Y-%m-%dT%H:%M:%S')
		jsondict['start_date'] = start_date.isoformat()
		end_date = start_date + + timedelta(seconds=jsondict['consumption_time_in_secs'])
		jsondict['end_date'] = end_date.isoformat()

		print "previous to insert 2"
		jsondict['uri'] = '%(machine_base_uri)s/%(deviceID)s/energy/%(start_date)s' % jsondict
		jsondict['consumption_type_ont'] = CONSUMPTION_TYPES[jsondict['consumption_type']]
		jsondict['userID'] == 'anonymous' if jsondict['userID'] == '' else jsondict['userID']

		print "previous to insert 3"
		rdf_template = '''@prefix propEnergy: <http://sweet.jpl.nasa.gov/2.3/propEnergy.owl#> .
@prefix rdfs:       <http://www.w3.org/2000/01/rdf-schema#> .
@prefix dul:        <http://www.loa-cnr.it/ontologies/DUL.owl#> .
@prefix ssn:        <http://purl.oclc.org/NET/ssnx/ssn#> .
@prefix loc:        <http://simile.mit.edu/2005/05/ontologies/location#> .
@prefix geo:        <http://www.w3.org/2003/01/geo/wgs84_pos#> .
@prefix dcterms:    <http://purl.org/dc/terms/> .
@prefix xsd:        <http://www.w3.org/2001/XMLSchema#> .
@prefix ucumext:    <http://dev.morelab.deusto.es/bizkaisense/ucum-extended.owl#> .
@prefix sc:         <http://www.morelab.deusto.es/ontologies/socialcoffee#> .

<%(uri)s> a ssn:Observation ;
	dcterms:date          "%(start_date)s"^^xsd:dateTime ;
	sc:hasStatus          %(consumption_type_ont)s ;
	ssn:observedBy        <%(machine_base_uri)s/%(deviceID)s> ;
	ssn:observedProperty  propEnergy:Energy ;
	ssn:observationResult <%(uri)s/sensoroutput> ;
	ssn:isObservableAt    <%(uri)s/interval> .

<%(uri)s/sensoroutput> a ssn:SensorOutput ;
	ssn:hasValue <%(uri)s/outputvalue> .

<%(uri)s/outputvalue> a ssn:ObservationValue ;
	dul:isClassifiedBy ucumext:watt-per-hour;
	dul:hasDataValue   "%(energy_consumption_Wh).3f"^^xsd:float .

<%(uri)s/interval> a ssn:TimeInterval ;
	ssn:hasIntervalDate "%(start_date)s"^^xsd:dateTime ;
	ssn:hasIntervalDate "%(end_date)s"^^xsd:dateTime .''' % jsondict

		if jsondict['userID'] != '-':
			rdf_template += '\n\n<%s>       sc:generatedBy        <%s/%s> .' % (jsondict['uri'], PERSON_BASE_URI, jsondict['userID'])

		#print "previous to insert"
		insert_by_post(rdf_template, GRAPH_URI, SPARUL_URI, USERNAME, PASSWD)
		#print "post  insert"
		return jsondict['uri']
	except:
		logger.error('Error saving JSON in RDF')


logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)
# Add the log message handler to the logger
handler = logging.FileHandler('ecoServer.log')
logger.addHandler(handler)

configFile = ConfigParser.RawConfigParser()
configFile.read('myConfig.cfg')
couch = couchdb.Server("http://"+configFile.get("myconfig", "dbServer"))
