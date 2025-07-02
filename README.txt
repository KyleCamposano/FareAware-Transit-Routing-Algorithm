What’s FareAware?

A modified Dikjstra’s Algorithm to calculate and sort public transportation by cheapest fare. Current navigation platforms like Google Maps prioritize fastest travel time or fewest transfers for public transportation but do not offer a clear way to sort or prioritize by fare cost.


How to Run (and dependencies)

1. Build the program.
   - Make sure you're using a C++11-compatible compiler
   - Run g++ -std=c++11 -o Final2 main.cpp TransitData.cpp Router.cpp Types.cpp
2. Prepare GTFS Data (see Files Needed)
   - Place gtfs_london.zip in a folder called data/
   - Ensure the zip contains standard GTFS files like stops.txt, trips.txt, stop_times.txt, etc.
   - Change filename to your .zip in main.cpp (line 28)
       -> if (!data.load("data", "gtfs_london.zip")) { // Change filename .zip
   - Select start and stop stations from stops.txt
3. Run
   - ./Final2 <start_stop_id|name|code> <end_stop_id|name|code>
      Example: ./Final2 ADELFAN2 CHEARICH


Files Needed (from GTFS)

* gtfs_london.zip containing:
   - stops.txt
   - trips.txt
   - stop_times.txt
   - fare_attributes.txt
   - fare_rules.txt


File Structure

* main.cpp : CLI entry point, input/output logic
* router.cpp/h ; Multi-criteria Dijkstra algorithm
* transitData.cpp/h : GTFS parsing, graph construction
* types.cpp/h : Struct definitions (State, Edge)
* data/ : Extracted GTFS files (from ZIP)


Cost Function 

* The score for a route is calculated as: Score = α × fare + β × time + γ × transfers
* α, β, and γ are weights defined per routing strategy
* Fare is based on either:
   - Flat per-segment ($0.50/stop)
   - Zone-based fare (if provided in GTFS)
* Time is calculated using GTFS stop_times.txt arrival times
* Transfers are counted when route_id changes between segments


How It Works

1.  Load GTFS data:
   - Unzips gtfs_london.zip to data/
   - Parses stops.txt, trips.txt, stop_times.txt, fare_rules.txt, etc.
   - Builds a graph where edges represent transit segments
2.  Resolve user input:
   - Start and end stops can be provided as stop IDs, names, or codes
3.  Run multi-objective Dijkstra:
   - For each routing mode, it finds the path with the lowest weighted score
   - Records fare, time, number of transfers, and the full path
4.  Post-processing:
   - Traverses the path to count actual route changes (transfers)
   - Recalculates the final score based on corrected transfer count
5.  Output:
   - Summary table of all routing strategies
   - Full list of stops visited in the Cheapest route
   - Segment-by-segment route transitions with transfer markings


Future Improvements

* Add real-time feeds (via GTFS-RT or APIs)
* Use dynamic fare
* Support for walking transfers and headway penalties
* GUI or map-based route visualization
* Using combinations of public transportations (bus, train, taxi, etc) to find the  cheapest route – extending the core idea of this algorithm to other assets