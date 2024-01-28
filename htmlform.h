const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="tr">
  <head>
    <title>Defne Güneş</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <meta charset="utf-8" />
    <style>
      body {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: flex-start;
        height: 100vh;
      }
      .sun-form {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        gap: 2em;
        width: 250px;
      }
      form > div {
        width: 100%;
      }
      #sunriseTime,
      #sunsetTime {
        width: 98.5%;
        padding-top: 5px;
        padding-bottom: 5px;
      }
      .submit-btn {
        width: 100%;
        padding: 1em 0em 1em 0em;
        font-size: 18px;
      }
      .directly-title {
        font-size: large;
        margin-top: 5em;
        margin-bottom: 1em;
      }
      .directly-btn-wrapper {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 9px;
        width: 250px;
      }
      .open-btn {
        width: 120px;
        padding: 1em 0em 1em 0em;
        background-color: darkgreen;
        color: white;
      }
      .close-btn {
        width: 120px;
        padding: 1em 0em 1em 0em;
        background-color: darkred;
        color: white;
      }
    </style>
  </head>

  <body>
    <h2>Defne Yapay Güneş</h2>
    <form action="/action.html" method="post" class="sun-form">
      <div>
        <label for="sunriseTime">Doğuş Saati:</label>
        <input
          id="sunriseTime"
          type="time"
          name="sunriseTime"
        />
      </div>
      <div>
        <label for="sunsetTime">Batış Saati:</label>
        <input
          id="sunsetTime"
          type="time"
          name="sunsetTime"
        />
      </div>
      <input type="submit" value="Kaydet" class="submit-btn" />
    </form>
    <p class="directly-title">Anında</p>
    <div class="directly-btn-wrapper">
      <form action="/action.html" method="post">
        <input type="hidden" id="directlyOpen" name="directlyOpen" value="0" />
        <input type="submit" value="Aç" class="open-btn" />
      </form>
      <form action="/action.html" method="post">
        <input
          type="hidden"
          id="directlyClose"
          name="directlyClose"
          value="1"
        />
        <input type="submit" value="Kapat" class="close-btn" />
      </form>
    </div>
  </body>
</html>
 )=====";