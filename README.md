# Highway Hierarchies

In this project I implemented part of the Highway Hierarchies algorithm. This algorithm is described by Dominik Schultes and Peter Sanders in "Highway Hierarchies Hasten Exact Shortest Path Queries", pulished in 2005. (can e found at https://link.springer.com/chapter/10.1007%2F11561071_51)

To hasten shortest paths queries, this algorithm defines the level of a road (similar to what is done in reality : small roads = lvl 0, national roads = lvl1 and highway = lvl3 for instance). Each road is assigned a level in the pre-caclculation step. Once this step is finished, each query ("What is the shortest path from s to t ?") is answered by running Dijkstra's algorithm on the new graph (augmented with road levels), with some restrictions to hasten it : when far from s and t, only high level roads can be used.

Proof that this algorithm is correct can be found in another paper by the same authors : https://pdfs.semanticscholar.org/670d/51961e403bf82aefda3711fc4f61ba5dcfad.pdf . Those proofs are quite interesting as they exploit all the specifities and properies of the Dijkstra's shortest path algorithm.

The authors experimentations also showed that the queries using the levels system are up to 2000x faster than a simple Dijkstra query.

I only implemented the precalculation part of the algorithm. I then applied it to a dataset representing most French roads (from http ://professionnels.ign.fr/route500). The results and a better description of the algorithm can be found in the pdf "presentation" (in French).
