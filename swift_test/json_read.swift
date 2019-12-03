import Foundation

struct HelloEntity:Codable {
  var hello: String
}

// Human構造体
struct Human:Codable {
    var id: Int
    var name: String
    var hobby: String
}


func getJSON() {
    let fileName = "hello.json"
    // ファイルを読み込みモードで開く
    let file = FileHandle(forReadingAtPath: fileName)!
    let data = file.readDataToEndOfFile()
    let contentString = String(data: data, encoding: .utf8)!
    file.closeFile()
    let jsonData = contentString.data(using: .utf8)!
    do {
        // ここの as! *** が重要
        let json = try! JSONDecoder().decode(HelloEntity.self, from: jsonData)

        print(json.hello)
        // => "world"
    }
}

getJSON()
