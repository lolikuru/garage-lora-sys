void listFiles(String path, String &html) {
  File root = LittleFS.open(path);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("Specified path is not a directory");
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    String fullPath = name;
    if (!fullPath.startsWith("/")) {
      if (path.endsWith("/")) fullPath = path + name;
      else if (path == "/") fullPath = "/" + name;
      else fullPath = path + "/" + name;
    }
    if (file.isDirectory()) {
      html += "<li><strong>" + name + "/</strong></li>";
      listFiles(fullPath, html);
    } else {
      html += "<li><a href='/download?file=" + fullPath + "'>" + name + "</a> (" + String(file.size()) + " bytes) ";
      html += "<a href='/edit?file=" + fullPath + "'>[Edit]</a></li>";
    }
    file = root.openNextFile();
  }
  root.close();
}

void FS_Browser_init() {
  // Route to display the list of files
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<h1>File Browser</h1><ul>";
    listFiles("/", html); // Recursively list files starting from the root
    html += "</ul>";
    request->send(200, "text/html", html);
  });

  // Route to download a file
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file")) {
      String filePath = request->arg("file");
      if (LittleFS.exists(filePath)) { // Check if the file exists
        request->send(LittleFS, filePath, "application/octet-stream", true); // Send the file
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "File not specified");
    }
  });

  // Route to edit a file
  server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file")) {
      String filePath = request->arg("file");
      if (LittleFS.exists(filePath)) {
        File file = LittleFS.open(filePath, "r");
        if (file) {
          String content = file.readString();
          file.close();

          // Form for editing the file
          String html = "<h1>Editing File: " + filePath + "</h1>";
          html += "<form method='POST' action='/save'>";
          html += "<input type='hidden' name='file' value='" + filePath + "'>";
          html += "<textarea name='content' rows='20' cols='80'>" + content + "</textarea><br>";
          html += "<input type='submit' value='Save'>";
          html += "</form>";
          request->send(200, "text/html", html);
        } else {
          request->send(500, "text/plain", "Failed to open file");
        }
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      request->send(400, "text/plain", "File not specified");
    }
  });

  // Route to save changes to a file
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasArg("file") && request->hasArg("content")) {
      String filePath = request->arg("file");
      String content = request->arg("content");

      File file = LittleFS.open(filePath, "w");
      if (file) {
        file.print(content);
        file.close();
        request->send(200, "text/plain", "File saved successfully");
      } else {
        request->send(500, "text/plain", "Failed to save file");
      }
    } else {
      request->send(400, "text/plain", "File or content not specified");
    }
  });

  // Start the server
  server.begin();
  Serial.println("Server started");
}
