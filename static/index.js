// This example adds a search box to a map, using the Google Place Autocomplete
// feature. People can enter geographical searches. The search box will return a
// pick list containing a mix of places and predicted search terms.
// This example requires the Places library. Include the libraries=places
// parameter when you first load the API. For example:
// <script src="https://maps.googleapis.com/maps/api/js?key=YOUR_API_KEY&libraries=places">
function initMap() {
    const map = new google.maps.Map(document.getElementById("map"), {
        center: {
            lat: 36.202569,
            lng: 36.160305,
        },
        zoom: 17,
        mapTypeId: "roadmap",
    });

    // Create the search box and link it to the UI element.
    const input = document.getElementById("pac-input");
    const searchBox = new google.maps.places.SearchBox(input);
    const infowindow = new google.maps.InfoWindow();

    map.controls[google.maps.ControlPosition.TOP_LEFT].push(input);

    // Bias the SearchBox results towards current map's viewport.
    map.addListener("bounds_changed", () => {
        searchBox.setBounds(map.getBounds());
    });

    var geocoder = new google.maps.Geocoder();
    map.addListener("click", (e) => {
        let latlng = e.latLng;
        geocoder
            .geocode({
                location: latlng
            })
            .then((response) => {
                console.log(response);

                let retdata = response.results;
                let lat = latlng.lat;
                let lon = latlng.lng;

                const url = "/api/location";
                let req = new XMLHttpRequest();
                req.onload = function(e) {
                    console.log(e);
                    if (req.status == 200) {
                        let resp = req.responseJSON;
                        console.log(resp);
                    }
                };
                req.onerror = function(e) {
                    console.log(e);
                };

                req.open("PUT", url);
                req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
                try {
                    req.send(JSON.stringify({
                        response: retdata,
                        lat: lat,
                        lon: lon
                    }));
                }
                catch (e) {
                    console.log(e);
                }

                if (response.results[0]) {
                    const marker = new google.maps.Marker({
                        position: latlng,
                        map: map,
                    });

                    infowindow.setContent(response.results[0].formatted_address);
                    infowindow.open(map, marker);
                }
                else {
                    window.alert("Adres bulunamadı!");
                }
            })
            .catch((e) => window.alert("Geocoder failed due to: " + e));
    });

    let markers = [];

    // Listen for the event fired when the user selects a prediction and retrieve
    // more details for that place.
    searchBox.addListener("places_changed", () => {
        const places = searchBox.getPlaces();

        if (places.length == 0) {
            return;
        }

        // Clear out the old markers.
        markers.forEach((marker) => {
            marker.setMap(null);
        });
        markers = [];

        // For each place, get the icon, name and location.
        const bounds = new google.maps.LatLngBounds();

        places.forEach((place) => {
            if (!place.geometry || !place.geometry.location) {
                console.log("Returned place contains no geometry");
                return;
            }

            const icon = {
                url: place.icon,
                size: new google.maps.Size(71, 71),
                origin: new google.maps.Point(0, 0),
                anchor: new google.maps.Point(17, 34),
                scaledSize: new google.maps.Size(25, 25),
            };

            // Create a marker for each place.
            markers.push(
                new google.maps.Marker({
                    map,
                    icon,
                    title: place.name,
                    position: place.geometry.location,
                })
            );

            if (place.geometry.viewport) {
                // Only geocodes have viewport.
                bounds.union(place.geometry.viewport);
            }
            else {
                bounds.extend(place.geometry.location);
            }
        });

        map.fitBounds(bounds);
    });
}

window.initMap = initMap;
