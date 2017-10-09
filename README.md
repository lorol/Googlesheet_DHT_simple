This project is based on https://github.com/electronicsguy/ESP8266

Other information and examples used:
https://mashe.hawksey.info/2014/07/google-sheets-as-a-database-insert-with-apps-script-using-postget-methods-with-ajax-example/
https://gist.github.com/bmcbride/7069aebd643944c9ee8b

  "Code.gs" is the server-side script.

  1. Enter sheet name where data is to be written below
      var SHEET_NAME = "Sheet1";       
  2. Run > setup

  3. Publish > Deploy as web app 
    - enter Project Version name and click 'Save New Version' 
    - set security level and enable service (most likely execute as 'me' and access 'anyone, even anonymously) 

  4. Copy the 'Current web app URL' and post this in your form/script action 

  5. Insert column names on your destination sheet matching the parameter names of the data you are passing in (exactly matching case)

  Script limits receiving to 800 rows, then deletes first 700 and keeps last 100
  Additionally, you can make another Sheet2 on same workbook for charts of last 20 rows to share it: Put to A1 cell:  =SORT(QUERY(Sheet1!A:D,"order by A desc limit 20"),1,0)

 6. Usage:
 
  - To write a row: https://script.google.com/macros/s/SCRIPT-ID/exec?par1=0.4&par2=2.1&par3=8   where par1 is the A2 header name, Timestamp is name of special A1 header which autoupdates
  - To Read [&delete] row: ?readrow=last[&delrow=yes]   last|-1(before last)|1(header row)|3(rd row),  delete except headers
    
  See Sheet1.png and Code.gs.png for more details.
