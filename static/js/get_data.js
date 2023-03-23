function getData() {
    wait = (wait + 1) % 10, $.ajax({
        url: "/get_data",
        method: "get",
        success: function(data) {
            $('#light').prop('disabled', data.auto_light).prop('checked', data.light);
            $('#auto_rotate').prop('disabled', data.const_rotate).prop('checked', data.auto_rotate);
            $('#const_rotate').prop('disabled', data.auto_rotate).prop('checked', data.const_rotate);
            $('#rotate_btn').prop('disabled', data.auto_rotate + data.const_rotate);
            $('#water_btn').prop('disabled', data.auto_water);
            $('#temper').text(data.temp);
            $('#hum_air').text(data.hum_air);
            $('#temper').text(data.hum_soil);
            $('#light_lvl').text(data.light_lvl);
            $('#sunset').text(data.sunset_str);
            $('#sunrise').text(data.sunrise_str);
        },
        error: function (jqXHR, exception) {
            if (jqXHR.status === 0) {
                alert('Not connect. Verify Network.');
            } else if (jqXHR.status == 404) {
                alert('Requested page not found (404).');
            } else if (jqXHR.status == 500) {
                alert('Internal Server Error (500).');
            } else if (exception === 'parsererror') {
                alert('Requested JSON parse failed.');
            } else if (exception === 'timeout') {
                alert('Time out error.');
            } else if (exception === 'abort') {
                alert('Ajax request aborted.');
            } else {
                alert('Uncaught Error. ' + jqXHR.responseText);
            }
        }
    })
}
var wait = 0;
$(document).ready(function() {
    getData(), setInterval("getData()", 1000)
});
