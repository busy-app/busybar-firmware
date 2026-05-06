import { mkdirSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, extname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const scriptDir = dirname(fileURLToPath(import.meta.url));
const frontendDir = resolve(scriptDir, '..');
const iconsDir = resolve(frontendDir, 'assets', 'icons', 'draw_tool');
const outputFile = resolve(frontendDir, 'generated', 'drawTool', 'icons.json');

const categories = readdirSync(iconsDir, { withFileTypes: true })
  .filter(entry => entry.isDirectory())
  .map(entry => entry.name)
  .sort((left, right) => left.localeCompare(right));

const iconsByCategory = Object.fromEntries(categories.map(category => {
  const categoryDir = resolve(iconsDir, category);
  const icons = readdirSync(categoryDir, { withFileTypes: true })
    .filter(entry => entry.isFile() && extname(entry.name).toLowerCase() === '.svg')
    .map(entry => ({
      id: entry.name.slice(0, -4),
      fileName: entry.name,
      path: `draw_tool/${category}/${entry.name}`
    }))
    .sort((left, right) => left.fileName.localeCompare(right.fileName));

  return [category, icons];
}));

mkdirSync(dirname(outputFile), { recursive: true });
writeFileSync(outputFile, `${JSON.stringify(iconsByCategory, null, 2)}\n`);
