bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  char pWG[pathWithGz.length() + 1];
  pathWithGz.toCharArray(pWG, sizeof(pWG));
  //    Serial.println(pWG);
  char p[path.length() + 1];
  path.toCharArray(p, sizeof(p));
  //    Serial.println(p);
  /**/
  if (sd.exists(pWG) || sd.exists(p)) { // If the file exists, either as a compressed archive, or normal
    if (sd.exists(pWG))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = sd.open(path, FILE_READ);                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    //    Serial.println("\tSent file: " + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}
