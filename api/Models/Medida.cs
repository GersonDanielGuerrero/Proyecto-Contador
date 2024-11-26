using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace api.Models
{
    public class Medida
    {
        public int potencia { get; set; }

        public long consumouWh { get; set; }

        public DateTime hora { get; set; }
    }
}