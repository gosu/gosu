
// Allison template
// Copyright 2007, 2008 Cloudburst, LLC. Licensed under the AFL 3. See the included LICENSE file.

var href_base = '%style_url%'.replace(/(.*\/).*/, '$1'); // inline js is good for something  

function $(id) {
    if (document.getElementById)
      elem = document.getElementById(id);
    else if ( document.all )
      elem = eval("document.all." + id);
    else
      return false;
    return elem;
}

  function toggle(id) {
    elem = $(id);
    elemStyle = elem.style;   
    if (elemStyle.display == "block") {
      elemStyle.display = "none"
    } else {
      elemStyle.display = "block"
    }
    return true;
  }

  function toggleText(id) {
    elem = $(id)
    if (m = elem.innerHTML.match(/(Hide)(.*)/)) {
      elem.innerHTML = "Show" + m[2];
    } else if (m = elem.innerHTML.match(/(Show)(.*)/)) {
      elem.innerHTML = "Hide" + m[2];
    }
    return true;
  }

function span(s, klass) {
  return '<span class="' + klass + '">' + s + '</span>';
}
  
function highlightSymbols() {
  pres = document.getElementsByTagName('pre');
  for(var i = 0; i < pres.length; i++) {
    pre = pres[i];
    spans = pre.getElementsByTagName('span');
    for(var k = 0; k < spans.length; k++) {
      span = spans[k];
      if (span.className.match(/ruby-identifier/)) {
        if (span.innerHTML.match(/^:/)) {
          span.className += " ruby-symbol";
        }
      }
    }
  }
}

 function hasClass(obj) {
     var result = false;
     if (obj.getAttributeNode("class") != null) {
         result = obj.getAttributeNode("class").value;
     }
     return result;
  }   

 function stripe() {
    var even = true;
    var color = "#e4ebed";
    var tables = document.getElementsByTagName('table');
    if (tables.length == 0) { return; }
    for (var h = 0; h < tables.length; h++) {
        var trs = tables[h].getElementsByTagName("tr");
        for (var i = 0; i < trs.length; i++) {
          var tds = trs[i].getElementsByTagName("td");
            for (var j = 0; j < tds.length; j++) {       
              if (hasClass(tds[j]) != "first") {                
              var mytd = tds[j];
              if (even) {
                mytd.style.backgroundColor = color;
              }         
            }
          }
          even =  ! even;
      }
    }
  }
  
function ajaxGet(url) {
  url = (href_base + url).replace('/./', '/')
  var req = false;

  if (window.ActiveXObject) {
    try {
      // stupid hack because IE7 disables local Ajax with the native xmlhttprequest object
      // for security purposes. Yet ActiveX still works. Thanks, Microsoft. I hate you. Die.
      req = new ActiveXObject("MSXML2.XMLHTTP.3.0");
    } catch (e) {
      try {
        /* IE 6 and maybe 5, don't know, don't care */
        req = new ActiveXObject("Msxml2.XMLHTTP");
      } catch (e) {
        try {
          req = new ActiveXObject("Microsoft.XMLHTTP");
        } catch (e) {
          req = false;
        }
      }
    }
  }
    
  /* real browsers */
  if (!req && window.XMLHttpRequest) {
    try {
      req = new XMLHttpRequest();
    } catch (e) {
      req = false;
    }
  } 
  
  if (req) {
    req.open('GET', url, false);
    req.send(null);
    return req.responseText;
  } else {
    return "Ajax error";  
  }
}


function addEvent(elm, evType, fn, useCapture) {
	if (elm.addEventListener) {
	  elm.addEventListener(evType, fn, useCapture);  
  	return true;
	} else if (elm.attachEvent) {
  	var r = elm.attachEvent('on' + evType, fn);  
	  return r;  
	} else {
  	elm['on' + evType] = fn;
	}
}

function insertIndices() {
  pages = ["class", "file", "method"]
  for (x in pages) { 
    $(pages[x]).innerHTML += ajaxGet('fr_' + pages[x] + '_index.html').replace(/(href=")/g, '$1' + href_base);
  }
  /* mouseoverify method links */
  links = $('method').getElementsByTagName('a');
  for (var x = 0; x < links.length; x++) {
    if (m = links[x].innerHTML.match(/(.*)\s\((.*)\)/)) {
      links[x].innerHTML = m[1] + '<br>';
      links[x].title = m[2];
    }
  }
  /* this is stupid */
  $('class').style.display = "block";
  $('file').style.display = "block";
  
  /* has to be here because IE7 does not guarantee the onLoad callback order */
  abbreviateIndicesInner(["class", "file"], 25, "a");
  /* same, linkTitle() depends on the class link list */
  linkTitle();
}

function abbreviateIndices() {
  var ids = ["defined_in", "child_of", "includes", "requires", "method", "methods"];
  abbreviateIndicesInner(ids, 25, 'a');
  abbreviateIndicesInner(ids, 25, 'span');
}

function abbreviateIndicesInner(indices, amount, tag) {
  for (var x = 0; x < indices.length; x++) { 
    var the_index = $(indices[x]);
    if (the_index) {
      links = the_index.getElementsByTagName(tag);
      for (var y = 0; y < links.length; y++) {
        var link = links[y];
        if (link.getElementsByTagName('span').length == 0 && link.getElementsByTagName('a').length == 0) {
          // avoid nesting
          link.innerHTML = link.innerHTML.replace(/<br>|\n/gi, '');
          link.title = link.innerHTML;
          link.innerHTML = abbreviate(link.innerHTML, amount) + '<br>';
        }
      }
    }
  }
}

function linkTitle() {
  
  /* grab the correct title element from the index */
  var index_page = ajaxGet('index.html');
  title_text = index_page.match(/<title>(.*)<\/title>/m)[1];
  document.title = title_text + " - " + document.title;
  var p = $('header').getElementsByTagName('p')[0]
  if (p.innerHTML.match(/^\s*$/)) {
    p.innerHTML = title_text;
  } else {
    p.innerHTML = title_text + ": " + p.innerHTML;
  }
  
  /* set the link properly */
  title_link = index_page.match(/<a\s+href="(.*?)"/)[1];
  var element = $('title');
  var item_type = "";
  var item_name = "";
  if (m = element.innerHTML.match(/(Class:|Module:|File:)\s*(.*)/)) {
    item_type = m[1];
    item_name = m[2];
  } else {
    item_name = element.innerHTML;
  }
  element.innerHTML = '<a href="' + href_base + title_link + '">' + item_type + " " + abbreviate(item_name, 45) + '</a>';
  element.getElementsByTagName('a')[0].title = item_name
  
  /* breadcrumb navigation */
  items = item_name.split("::");
  items_new = item_name.split("::");
  file_links = $('class').getElementsByTagName('a');
  for (var x = 0; x < items.length - 1; x++ ){
    var item = items[x];
    link = ("/classes/" + items.slice(0,x).join("/") + "/" + item + ".html").replace('//', '/');
    regex = new RegExp(RegExp.escape(link) + '$');
    for (var y = 0; y < file_links.length; y++) {
      if (file_links[y].href.match(regex)) {
         items_new[x] = '<a href="' + href_base + link + '">' + item + '</a>';
         break;
      }
    }  
  }
  $('item_name').innerHTML = item_type + ' ' + items_new.join(" :: ");
}

function abbreviate(s, size) {
  // Try to get rid of path and extension in any case. These are not
  // even interesting if there is enough space.
  
  s = s.replace(/\..*$/, '');
  s = s.replace(/.*\//g, '');
  
  // Now this is a blatant hack.
  
  if (s == 'gosu')
    return ''; // Noone will ever see the link, harhar!

  while (s.length > size) {
    var old_s = s;
    s = s.replace(/\s|\n/mg, '');
    s = s.replace(/([A-Z])[a-z]+/m, '$1');
    if (!s || old_s == s) {
      return "..." + s.substring(s.length - size, s.length);
    }
  }
  return s;
}

function disableSubmit(event) {
  var keyCode = event.keyCode ? event.keyCode : event.which ? event.which : event.charCode;
  if (keyCode == 13) {
    return false;
  } else {
    return true;
  }
}
  
function filterList(id, s, event) {
  
  /* some weak escaping */
  s = s.replace(/[^\w\d\.\_\-\/\:\=\[\]\?\!]/g, '');
  s = RegExp.escape(s);
  
  var show_all = false;
  if (s.match(/^\s*$/)) {
    show_all = true;
  }
  
  links = $(id).getElementsByTagName('a')
  regex = new RegExp(s, 'i');
  
  for (var x = 0; x < links.length; x++) {
    var link = links[x];
    if (show_all) {
      link.style.display = 'inline';
    } else {
       if (link.innerHTML.match(regex)) {        
         link.style.display = 'inline';
       } else {
         link.style.display = 'none';
       }
    }
  }
  return true;
}

RegExp.escape = function(text) {
  if (!arguments.callee.sRE) {
    var specials = ['/', '.', '*', '+', '?', '|', '(', ')', '[', ']', '{', '}', '\\\\'];
    arguments.callee.sRE = new RegExp(
      '(\\\\' + specials.join('|\\\\') + ')', 'g'
    );
  }
  return text.replace(arguments.callee.sRE, '\\\\$1');
}

function hacks() {
  // show the spacer if necessary, 
  divs = document.getElementsByTagName('div');
  for(var x = 0; x < divs.length; x++) {
    if (divs[x].className && divs[x].className.match(/top/)) {
      document.getElementById('spacer').style.display = 'block';
    }
  }
  // remove extra colons from tables
  tds = document.getElementsByTagName('td');
  for(var x = 0; x < tds.length; x++) {
    str = tds[x].innerHTML
    if (str.charAt(str.length - 1) == ":") {
      tds[x].innerHTML = str.slice(0, str.length - 1)
    }
  }
}

addEvent(window, 'load', insertIndices, false);
addEvent(window, 'load', abbreviateIndices, false);
addEvent(window, 'load', stripe, false);
addEvent(window, 'load', highlightSymbols, false);
addEvent(window, 'load', hacks, false);
