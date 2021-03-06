<?php


//Communications & Marketing format for times
function iit_display_hours_format_time ($time){ 
	$time = str_replace(':00','',$time);
	$time = str_replace('am',' a.m.',$time);
	$time = str_replace('pm',' p.m.',$time);
	return $time;
}

function iit_display_hours_get_and_format_todays_date(){
  $dateFormat="l, F j";
  $today = date($dateFormat);
  return $today;
}
//retrieve JSON data from a Google Calendar (public)
function iit_display_hours_get_calendar_data($calendar, $dateToGet=0){
  $debug=false;
  $key = getenv('GOOGLE_API'); // duritalservices@iit.edu Google API key
  if(($key==NULL)||($key=="")){
    trigger_error('Google API key not found', E_USER_NOTICE);
  }  
  $APIformat="Y-m-d";
  $timeMin = date($APIformat,time()+$dateToGet) . 'T06:00:00.000Z';
  $timeMax = date($APIformat,time()+$dateToGet) . 'T22:00:00.000Z'; // change to 10pm. Sunday times weren't showing up due to Google cal weirdness.
  $url='https://www.googleapis.com/calendar/v3/calendars/' . $calendar . '/events?singleEvents=true&orderby=startTime&timeMin=' . 
      $timeMin . '&timeMax=' . $timeMax . '&maxResults=1&key=' . $key;
    //this works more reliably than only getting one event
    //google api pads date, was causing problems with returning yesterday's event. Instead of using midnight and 11:59 for range, use noon-1pm
  if ($debug){
    print $url . "<br/>";
  }
  $jsonFile = file_get_contents($url);
  if (!$jsonFile) {
      trigger_error('NO DATA returned from url.', E_USER_NOTICE);
  }
  else {
    // convert the string to a json object
    $jsonObj = json_decode($jsonFile);
    $dateData = $jsonObj->items;
    return $dateData;
  }
}

function iit_display_hours_check_if_open($item){   
  $debug=false;
  $now=time();
  
  if (isset($item->summary)){ 
    $title = $item->summary;
    if (stripos($title,"closed")!==false){ // don't look at times if 'closed' in title
      return 0;
    }
  }
  
  if (isset($item->start->dateTime)){ // non 24-hour event
      $unixStart=strtotime(substr($item->start->dateTime, 0,16));
      $unixEnd=strtotime(substr($item->end->dateTime, 0,16));
  }

  else{ // all day event
    $unixStart=strtotime(substr($item->start->date, 0,16));
    $unixEnd=strtotime(substr($item->end->date, 0,16));
  }
  if ($debug){
    print_r($item);
    print "<br/>now=$now start=$unixStart end=$unixEnd";
  }

  if ( ($now < $unixStart) || ($now > $unixEnd) ){
    $isOpen = 0;
  }
  else {
    $isOpen = 1;
  }      
  
  return $isOpen;
}

function iit_display_hours_format_open_msg($isOpen){
  if ($isOpen<=0){
    $openMsg="<span class=\"closed\">CLOSED</span>";   
  }          
  else {
    $openMsg="<span class=\"open\">OPEN</span>";
  }
  return $openMsg;
}

function iit_display_hours_format_hours_data($dateData){// default is to use Galvin and today's Unix date
  $msg="no data available";
  $timeFormat="g:ia";
  
// error gracefully if no data
    if (count($dateData)<=0){
      return $msg;
    }
    else{
      $item = $dateData[0]; // no need to loop. just get first object
    }     
    $title = $item->summary;

    if (stripos($title,"closed")===false) { // library open (verify identical FALSE to avoid "false false")

        // Google Calendar API v3 uses the date field if event is a full day long, or the dateTime field if it is less than 24 hours  
      if (isset($item->start->dateTime)){ // non 24-hour event
          $tmpStart=strtotime(substr($item->start->dateTime, 0,16));
          $tmpEnd=strtotime(substr($item->end->dateTime, 0,16));
      }

      else{ // all day event
        $tmpStart=strtotime(substr($item->start->date, 0,16));
        $tmpEnd=strtotime(substr($item->end->date, 0,16));
      }
      
      $startTime = iit_display_hours_format_time(date($timeFormat,$tmpStart));
      $endTime = iit_display_hours_format_time(date($timeFormat,$tmpEnd));
      
      $msg=iit_display_hours_format_todays_hours($startTime, $endTime);

      return $msg; // return hours info
    } // end library open

    // library is closed
    else {
      return $title;
    }
        
}// end function


function iit_display_hours_format_todays_hours($startTime,$endTime, $msgtomorrow){
  if ( ($endTime=="12 a.m.") && ($startTime=="12 a.m.") ){ // don't use 12am time to avoid confusion
    return "Open 24 hours";
  }
  else if  ( ($endTime=="12 a.m.") && ($startTime!="12 a.m.") && ($msgtomorrow=="Open 24 hours") ){
    return "Open $startTime - Overnight";
  }
  else if  ( ($endTime=="12 a.m.") && ($startTime!="12 a.m.") ){
    return "Open $startTime - Midnight";
  }
  else if  (($endTime!="12 a.m.") && ($startTime=="12 a.m.")){
    return "Open until $endTime";
  }  
  else { // normal 
    return "$startTime - $endTime"; // eg: Saturday 8:30am-5pm
  }  
}
function iit_display_hours_format_message($incluldeOpenMsg){
  $debug=false;
  $galvinCal="iit.edu_8l0d8qd4qtfn7skmgkiu55uv58%40group.calendar.google.com";
  $dataObj=iit_display_hours_get_calendar_data($galvinCal);
  $closed="";

  if (count($dataObj)>0){
    if ($debug){
      print_r($dataObj[0]);
    }
    $message = "<p>" . iit_display_hours_format_hours_data($dataObj) . " " . l(t('more...'), '/hours',array('attributes'=>array('class'=>'page-hide bump-right'))) . "</p>";
    
    if ($incluldeOpenMsg>0) {// won't use this flag for hours page
      $isOpen = iit_display_hours_check_if_open($dataObj[0]);
      $openMsg = iit_display_hours_format_open_msg($isOpen);
      $message .= "<p class=\"clear\">Currently: $openMsg</p>";
    }
  }
  else{
    $message .= "<p>Library hours cannot be displayed at this time.</p>";
  }

  return $message;  
  
}

