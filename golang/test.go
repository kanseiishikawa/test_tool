package main

import (
    "html/template"
    "io"
    "net/http"
    "os"
    "fmt"
)

//「/save」用のハンドラ
func saveHandler(w http.ResponseWriter, r *http.Request) {
    //MultipartReaderを用いて受け取ったファイルを読み込み
    reader, err := r.MultipartReader()

    //エラーが発生したら抜ける
    if err != nil {
        http.Error(w, err.Error(), http.StatusInternalServerError)
        return
    }

    //forで複数ファイルがある場合に、すべてのファイルが終わるまで読み込む
    for {
        part, err := reader.NextPart()
		fmt.Println( part )
        if err == io.EOF {
            break
        }

        //ファイル名がない場合はスキップする
        if part.FileName() == "" {
            continue
        }

        //uploadedfileディレクトリに受け取ったファイル名でファイルを作成
        uploadedFile, err := os.Create("/Users/kansei/Desktop/test_tool/golang/" + part.FileName())
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            uploadedFile.Close()
            redirectToErrorPage(w,r)
            return
        }

        //作ったファイルに読み込んだファイルの内容を丸ごとコピー
        _, err = io.Copy(uploadedFile, part)
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            uploadedFile.Close()
            redirectToErrorPage(w,r)
            return
        }
    }
    //uploadページにリダイレクト
    http.Redirect(w,r,"/upload",http.StatusFound)
}

//「/upload」用のハンドラ
func uploadHandler(w http.ResponseWriter, r *http.Request) {
    var templatefile = template.Must(template.ParseFiles("/Users/kansei/Desktop/test_tool/golang/upload.html"))
    templatefile.Execute(w, "upload.html")
}

//「/errorPage」用のハンドラ
func errorPageHandler(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w,"%s","<p>Internal Server Error</p>")
}

//errorが起こった時にエラーページに遷移する
func redirectToErrorPage(w http.ResponseWriter, r *http.Request) {
    http.Redirect(w,r,"/errorPage",http.StatusFound)
}

func main() {
    //ハンドラの登録
    http.HandleFunc("/upload", uploadHandler)
    http.HandleFunc("/save",saveHandler)
    http.HandleFunc("/errorPage",errorPageHandler)
    //サーバーの開始
    http.ListenAndServe(":8080", nil)
}
