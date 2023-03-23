function CreateRequest()
    {
        var Request = false;
        if (window.XMLHttpRequest)
        {
            Request = new XMLHttpRequest();
        }
        else if (window.ActiveXObject)
        {
            try
            {
                 Request = new ActiveXObject("Microsoft.XMLHTTP");
            }
            catch (CatchException)
            {
                 Request = new ActiveXObject("Msxml2.XMLHTTP");
            }
        }
        if (!Request)
        {
            alert("Невозможно создать XMLHttpRequest");
        }
        return Request;
    }

function sendData(elem)
    {
        var Request = CreateRequest();
        if (!Request)
        {
            return;
        }
        var chbox;
        chbox = document.getElementById(elem);
        let sendingData = {
            dat: chbox.checked
        }
        let data = JSON.stringify(sendingData);
        Request.open('POST', 'http://172.16.102.41:80/' + elem, true);
        Request.setRequestHeader('Content-type', 'application/json; charset=UTF-8');
        Request.send(data);
    }