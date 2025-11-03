package main

import (
	"html/template" // Note: html/template, NOT text/template
	"os"
)

// Define the data structure to pass to the template
type PageData struct {
	Title   string
	Message string
	Items   []string
}

func main() {
	// 1. Define the template as a string.
	// Notice the {{.FieldName}} syntax.
	const tmpl = `
<!DOCTYPE html>
<html>
<head>
	<title>{{.Title}}</title>
</head>
<body>
	<h1>{{.Message}}</h1>
	<p>Here are your items:</p>
	<ul>
		{{range .Items}}
			<li>{{.}}</li>
		{{end}}
	</ul>
</body>
</html>`

	// 2. Create the data to inject
	data := PageData{
		Title:   "My Go HTML Page",
		Message: "Hello from the Go standard library!",
		Items:   []string{"Item 1", "Item 2", "Item 3"},
	}

	// 3. Parse the template
	t, err := template.New("webpage").Parse(tmpl)
	if err != nil {
		panic(err) // In real code, handle this error
	}

	// 4. "Execute" the template, writing the output to Standard Out
	// This merges the data with the template.
	err = t.Execute(os.Stdout, data)
	if err != nil {
		panic(err)
	}
}
