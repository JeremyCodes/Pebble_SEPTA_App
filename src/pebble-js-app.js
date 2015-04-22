var RADIUS = 1.0; //Miles
var DEBUG_ON_PC = false;

var Stations = {"stations" : [
  {"name": "Secane", "lookup_name": "Secane", "lat": "39.91574", "long": "-75.30986"},
  {"name": "Suburban", "lookup_name": "Suburban%20Station","lat": "39.95425", "long": "-75.167"},
]};

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

  //http://www3.septa.org/hackathon/Arrivals/Suburban%20Station/5/

function locationSuccess(pos) {
  console.log("user location retrieved. (" + pos.coords.latitude + ", " + pos.coords.longitude + ")");
  
  //for testing
  if (DEBUG_ON_PC)
  {
    pos.coords.latitude = 39.9529899;
    pos.coords.longitude = -75.1684441;
  }
  
  // determine which station you are currently located near.
  var station = getNearestStation(pos);
  
  if (station !== null)
  { 
    // Construct URL  
  var url = 'http://www3.septa.org/hackathon/Arrivals/' + station.lookup_name + '/1/';
  //  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
  //      pos.coords.latitude + '&lon=' + pos.coords.longitude;
  
    // Send request to OpenWeatherMap
    xhrRequest(url, 'GET', 
      function(responseText) {
        // responseText contains a JSON object with train info
        try {
          
          var json = JSON.parse(responseText);
          
          /*
          // Temperature in Kelvin requires adjustment
          var temperature = (((Math.round(json.main.temp - 273.15)) * 1.8) + 32);
          console.log('json.main.temp is ' + json.main.temp);
          console.log('Temperature is ' + temperature);
    
          // Conditions
          var conditions = json.weather[0].main;      
          console.log('Conditions are ' + conditions);
          */
          // Assemble dictionary using our keys
          var dictionary = {
            'KEY_STATION': station.name,
            'KEY_TIME': "4:34p",
            'KEY_TARDINESS': "6 min"
          };
          
          // Send to Pebble
          Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log('Train info sent to Pebble successfully!');
            },
            function(e) {
              console.log('Error sending train info to Pebble!');
            }
          );
        }
        catch (e) {
          console.log("Exception caught in xhrRequest: "+ e);
        }
      }
    );
  } 
  else // Out of septa station range send that note back.
  {
          var dictionary = {
            'KEY_STATION': "Out of Range",
            'KEY_TIME': "",
            'KEY_TARDINESS': ""
          };
            // Send to Pebble
          Pebble.sendAppMessage(dictionary,
            function(e) {
              console.log('Train info sent to Pebble successfully!');
            },
            function(e) {
              console.log('Error sending train info to Pebble!');
            }
          );
  }
}

function getNearestStation(pos) {
  var return_value = null;
  
  for (var i = 0; i < Stations.stations.length; i++)
  {
    var distance = mathGeoDistance( Stations.stations[i].lat, Stations.stations[i].long, pos.coords.latitude, pos.coords.longitude, true );
      
    if (distance <= RADIUS)
    {
      console.log("nearest station found: " + Stations.stations[i].name);
      return Stations.stations[i];
    }
  }
  console.log("No local station found!");
  return return_value;
}

function mathGeoDistance( p_lat1, p_lng1, p_lat2, p_lng2, p_miles ) {
    var M_PI = 3.14;
    var pi80 = M_PI / 180;
    var lat1 = p_lat1 * pi80;
    var lng1 = p_lng1 * pi80;
    var lat2 = p_lat2 * pi80;
    var lng2 = p_lng2 * pi80;

    var r = 6372.797; // mean radius of Earth in km
    var dlat = lat2 - lat1;
    var dlng = lng2 - lng1;
    var a = Math.sin(dlat / 2) * Math.sin(dlat / 2) + Math.cos(lat1) * Math.cos(lat2) * Math.sin(dlng / 2) * Math.sin(dlng / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var km = r * c;

    return (p_miles ? (km * 0.621371192) : km);
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getTrain() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    
    // Get the initial train
    getTrain();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getTrain();
  }                     
);