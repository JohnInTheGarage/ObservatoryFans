package main

import (
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	"github.com/tarm/serial"
)

//=====================================
func newFile(now time.Time) (*os.File, error) {

	filename := fmt.Sprintf("/media/usbstick/aht20data/values_%d%02d%02d.csv",
		now.Year(), now.Month(), now.Day())

	file, err := os.OpenFile(filename, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return nil, fmt.Errorf("failed to open file %s: %w", filename, err)
	}
	return file, nil
}

//=====================================
// checkStopFile checks if the "stop" file exists and returns true if it does.
func checkStopFile() bool {
	_, err := os.Stat("stop")
	return !os.IsNotExist(err)
}

func main() {
	c := &serial.Config{Name: "/dev/ttyACM0", Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	defer s.Close() // Ensure the serial port is closed when the program exits

	buf := make([]byte, 128)
	var thisFile *os.File
	lastDay := -1
	partialMessage := ""
	delimiter := "\r\n" // Assuming messages end with carriage return and newline.

	for {
		date := time.Now()
		if date.Day() != lastDay {
			if lastDay != -1 {
				thisFile.Close()
			}
			lastDay = date.Day()
			thisFile, err = newFile(date)
			fmt.Fprintf(thisFile, "%s", "Time,Inside Temp,Inside Rh,Outside Temp,Outside Rh,Fans On\r\n")
			if err != nil {
				fmt.Println("Error:", err)
				return
			}
			defer thisFile.Close()
		}

		n, err := s.Read(buf)
		if err != nil {
			log.Fatal(err)
		}
		readString := string(buf[:n])

		partialMessage += readString
		
		for strings.Contains(partialMessage, delimiter){
			parts := strings.SplitN(partialMessage, delimiter, 2)
			completeMessage := parts[0]
			partialMessage = parts[1]

			fmt.Fprintf(thisFile, "%02d:%02d,", date.Hour(), date.Minute())
			fmt.Fprintf(thisFile, "%s\r\n", completeMessage)
		}

		if checkStopFile() {
			fmt.Println("Stop file detected. Exiting program.")
			if thisFile != nil {
				thisFile.Close()
			}
			break
		}


	}

}
