//  1. Enter sheet name where data is to be written below
        var SHEET_NAME = "Sheet1";       
//  2. Run > setup
//
//  3. Publish > Deploy as web app 
//    - enter Project Version name and click 'Save New Version' 
//    - set security level and enable service (most likely execute as 'me' and access 'anyone, even anonymously) 
//
//  4. Copy the 'Current web app URL' and post this in your form/script action 
//
//  5. Insert column names on your destination sheet matching the parameter names of the data you are passing in (exactly matching case)
//
//  Script limits receiving to 800 rows, then deletes first 700 and keeps last 100
//  Additionally, you can make another Sheet2 on same workbook for charts of last 20 rows to share it: Put to A1 cell:  =SORT(QUERY(Sheet1!A:D,"order by A desc limit 20"),1,0)
//
//  Add row query from Internet: https://script.google.com/macros/s/SCRIPT-ID/exec?par1=0.4&par2=2.1&par3=8   if par1 is the A2 header name, Timestamp is A1 header
//  show[&delete] row query: Use this parameter: ?readrow=last[&delrow=yes]   last|-1(before last)|1(header row)|3(rd row)  delete except headers
//    
 
var SCRIPT_PROP = PropertiesService.getScriptProperties(); // new property service
 
// If you don't want to expose either GET or POST methods you can comment out the appropriate function
function doGet(e){
  return handleResponse(e);
}
 
function doPost(e){
  return handleResponse(e);
}
 
function handleResponse(e) {
  // shortly after my original solution Google announced the LockService[1]
  // this prevents concurrent access overwritting data
  // [1] http://googleappsdeveloper.blogspot.co.uk/2011/10/concurrency-and-google-apps-script.html
  // we want a public lock, one that locks for all invocations
  var lock = LockService.getPublicLock();
  lock.waitLock(30000);  // wait 30 seconds before conceding defeat.
   
  try {
    // next set where we write the data - you could write to multiple/alternate destinations
    var doc = SpreadsheetApp.openById(SCRIPT_PROP.getProperty("key"));
    var sheet = doc.getSheetByName(SHEET_NAME);
     
    // we'll assume header is in row 1 but you can override with header_row in GET/POST data
    var headRow = e.parameter.header_row || 1;
    var headers = sheet.getRange(1, 1, 1, sheet.getLastColumn()).getValues()[0];
	
	// query with /exec?readrow=-2|5|last  to retreive the row values.
	if (e.parameter.readrow) {
		var deleted = false;
        var showrow = sheet.getLastRow();
        var askrow = Number(e.parameter.readrow);
        if (askrow < 0) showrow = showrow + askrow;
        else if (askrow > 0) showrow = askrow;
        var showval = sheet.getRange(showrow, 1, showrow, sheet.getLastColumn()).getValues()[0];
        if ((e.parameter.delrow) && (showrow != 1)) { // can delete except first
          sheet.deleteRow(showrow);
		  deleted = true;
		}
        return ContentService
			.createTextOutput(JSON.stringify({"result":"success", "row": showrow, "values": showval, "deleted": deleted}))
			.setMimeType(ContentService.MimeType.JSON);

	} else {   
		var nextRow = sheet.getLastRow()+1; // get next row
		var row = []; 
        if (nextRow > 800){
          sheet.deleteRows(2, nextRow - 100); // limit to 800 rows max
          nextRow = 100;
        }
		// loop through the header columns
		for (i in headers){
			  if (headers[i] == "Timestamp"){ // special case if you include a 'Timestamp' column
				row.push(new Date());
			  } else { // else use header name to get data
				row.push(e.parameter[headers[i]]);
			  }
		}
		// more efficient to set values as [][] array than individually
		sheet.getRange(nextRow, 1, 1, row.length).setValues([row]);
		// return json success results - add / populate
		return ContentService
			  .createTextOutput(JSON.stringify({"result":"success", "row": nextRow}))
			  .setMimeType(ContentService.MimeType.JSON);
	}	     
		  
  } catch(e){
    // if error return this
    return ContentService
          .createTextOutput(JSON.stringify({"result":"error", "error": e}))
          .setMimeType(ContentService.MimeType.JSON);
  } finally { //release lock
    lock.releaseLock();
  }
}
 
function setup() {
    var doc = SpreadsheetApp.getActiveSpreadsheet();
    SCRIPT_PROP.setProperty("key", doc.getId());
}
