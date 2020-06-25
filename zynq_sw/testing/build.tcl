set app_name "horizon_detection"

if [file exist ../workspace] {
    setws ../workspace
} else {
    puts "Error, couldn't find the workspace directory." 
    puts "You should be running this from the testing directory and"
    puts "you should have run the project setup script to generate the app."
    exit
}

app build -name $app_name
