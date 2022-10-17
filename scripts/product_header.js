/*
    liblifx - product_header.js
    NodeJS script that creates a lifx_product_info_t array from LIFX's official JSON at
    https://github.com/LIFX/products/blob/master/products.json
*/

var fs = require("fs");
var productlist = JSON.parse(fs.readFileSync("products.json"));
var output = "const static lifx_product_info_t lifx_products[] = \n";
output += "{";
for (var i = 0; i < productlist[0].products.length; i++) {
    var product = productlist[0].products[i];
    output += "    {\n";
    output += "        .id = " + product.pid + ",\n";
    output += "        .product_name = \"" + product.name + "\",\n";
    if (product.features.hev)
        output += "        .hev = true,\n";
    if (product.features.color)
        output += "        .color = true,\n";
    if (product.features.chain)
        output += "        .chain = true,\n";
    if (product.features.matrix)
        output += "        .matrix = true,\n";
    if (product.features.relays)
        output += "        .relays = true,\n";
    if (product.features.buttons)
        output += "        .buttons = true,\n";
    if (product.features.infrared)
        output += "        .infrared = true,\n";
    if (product.features.multizone)
        output += "        .multizone = true,\n";
    if (product.features.extended_multizone)
        output += "        .extended_multizone = true,\n";
    if (product.features.temperature_range != null) {
        output += "        .temp_min = " + product.features.temperature_range[0] + ",\n";
        output += "        .temp_max = " + product.features.temperature_range[1] + ",\n";
    }
    output += "    },\n";
}
output += "};";
fs.writeFileSync("products.txt", output);
