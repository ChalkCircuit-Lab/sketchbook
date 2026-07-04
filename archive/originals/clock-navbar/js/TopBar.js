updateDateOnTopBar();


function updateDateOnTopBar(){
    var date = getFormatedDate();
    // console.log(date);
    // console.log(document.getElementsByClassName("TopBar-module--longdate")[0]);
    document.getElementsByClassName("TopBar-module--longdate")[0].getElementsByTagName("p")[0].innerHTML = date;
}

function getFormatedDate(){
    // Format Example: SUNDAY | DEC 3RD, 2023
    
    var date = new Date();
    var day = date.getDay();
    var month = date.getMonth();
    var dayOfMonth = date.getDate();
    var year = date.getFullYear();

    var dayString = "";
    var monthString = "";
    var dayOfMonthString = "";
    var yearString = "";

    switch(day){
        case 1:
            dayString = "MONDAY";
            break;
        case 2:
            dayString = "TUESDAY";
            break;
        case 3:
            dayString = "WEDNESDAY";
            break;
        case 4:
            dayString = "THURSDAY";
            break;
        case 5:
            dayString = "FRIDAY";
            break;
        case 6:
            dayString = "SATURDAY";
            break;
        case 0:
            dayString = "SUNDAY";
            break;
    }

    switch(month){
        case 0:
            monthString = "JAN";
            break;
        case 1:
            monthString = "FEB";
            break;
        case 2:
            monthString = "MAR";
            break;
        case 3:
            monthString = "APR";
            break;
        case 4:
            monthString = "MAY";
            break;
        case 5:
            monthString = "JUN";
            break;
        case 6:
            monthString = "JUL";
            break;
        case 7:
            monthString = "AUG";
            break;
        case 8:
            monthString = "SEP";
            break;
        case 9:
            monthString = "OCT";
            break;
        case 10:
            monthString = "NOV";
            break;
        case 11:
            monthString = "DEC";
            break;
    }

    switch(dayOfMonth){
        case 1:
            dayOfMonthString = "1ST";
            break;
        case 2:
            dayOfMonthString = "2ND";
            break;
        case 3:
            dayOfMonthString = "3RD";
            break;
        case 21:
            dayOfMonthString = "21ST";
            break;
        case 22:
            dayOfMonthString = "22ND";
            break;
        case 23:
            dayOfMonthString = "23RD";
            break;
        case 31:
            dayOfMonthString = "31ST";
            break;
        default:
            dayOfMonthString = dayOfMonth + "TH";
            break;
    }

    yearString = year;
    return dayString + " | " + monthString + " " + dayOfMonthString + ", " + yearString;
}


