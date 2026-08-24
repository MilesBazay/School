const panel_width = 300;
const panel_height = 75;



window.addEventListener("load", makePhaseOne);

// Lotsa magic numbers below...

function makePhaseOne() {
    const svg = d3.select("body")
        .append("svg")
        .attr("width", panel_width)
        .attr("height", panel_height);

    svg.append("rect")
        .attr("x", 20)
        .attr("y", 20)
        .attr("width", 200)
        .attr("height", 50)
        .attr("fill", "yellow")
        .attr("stroke", "black")
        .attr("rx", 10)
        .attr("ry", 10);

    svg.append("text")
        .attr("x", 120)  // Center text in the rectangle
        .attr("y", 50)
        .attr("text-anchor", "middle")  // Center horizontally
        .attr("dominant-baseline", "middle")  // Center vertically
        .attr("fill", "black")
        .attr("font-size", "20px")
        .attr("font-family", "Arial, sans-serif")
        .text("Languages");
    
    
    d3.json("languages-simpler.json").then(data => {
        
            d3.select("body")
                .append("ul")
                .selectAll("li")
                .data(data.languages)
                .enter()
                .append("li")
                .style("font-size", "18px")
                .text( d => `${d.name}: ${d.paradigm[0]}` );
    });
    
}
